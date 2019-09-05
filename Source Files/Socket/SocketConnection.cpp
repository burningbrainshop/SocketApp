#include "stdafx.h"
#include "SocketConnection.h"


// 組合連線名稱 : [IP] / [IP:Port] 
String^ SocketConnection::GetConnectionName(bool paramSingle)
{
	String^ tempName;

	try
	{
		if (paramSingle)
			tempName = remoteIP;
		else
			tempName = remoteIP + ":" + remotePort;

		return tempName;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete tempName;
	}
}


// -------------------------------------------- Public Functions -------------------------------------------//

// 建構子
SocketConnection::SocketConnection(ISocket ^paramServer)
{
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// 取得連線
		sckConnect = paramServer->GetAcceptSocket();
		RecvInvoker = gcnew MessageCallback(safe_cast<TcpServer^>(paramServer), &TcpServer::ReceivedCallback);
		// 設定參數
		localIP = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Address->ToString();
		remoteIP = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Address->ToString();
		localPort = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Port;
		remotePort = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Port;
		isConnected = true;

		writerStream = gcnew BinaryWriter(gcnew NetworkStream(sckConnect, FileAccess::Write));
		stateObj->workSocket = sckConnect;

		// 建立並啟動傳送訊息佇列內訊息的執行緒
		sendThread = gcnew Thread(gcnew ThreadStart(this, &SocketConnection::SendDataInQueue));
		sendThread->IsBackground = true;
		sendThread->Priority = ThreadPriority::Highest;
		sendThread->Start();

		// 進行非同步讀取
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &SocketConnection::ReceivedData), stateObj);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 傳送訊息 : 先放入佇列等待傳送
void SocketConnection::SendData(array<Byte> ^paramData)
{
	try
	{
		if (paramData != nullptr)
		{
			Monitor::Enter(sendQueue);
			try
			{
				sendQueue->Enqueue(paramData);
			}
			finally
			{
				Monitor::Exit(sendQueue);
			}

			SetSignal(SendSignal);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 以同步方式(Blocking)發送訊息，會等待訊息回應
ISocketMessage ^SocketConnection::ChatData(array<Byte> ^paramData)
{
	ISocketMessage ^responseMsg;
	DateTime stopTime;
	int waitTime;

	try
	{
		Monitor::Enter(this);
		try
		{
			if (isConnected)
			{
				stopTime = DateTime::Now.AddSeconds(chatTimeout);
				if (stopTime > DateTime::Now)
				{
					responseMsg = nullptr;
					ackMessage = nullptr;
					// 重設執行緒訊號
					ChatAckSignal->Reset();
					isChat = true;
					// 傳送資料至遠端點, 並等待回應訊息
					if (writerStream != nullptr)
					{
						writerStream->Write(paramData);
						writerStream->Flush();
						waitTime = safe_cast<int>((safe_cast<TimeSpan^>(safe_cast<DateTime^>(stopTime)->Subtract(DateTime::Now)))->TotalMilliseconds);
						// 等待訊息回傳訊號
						if (waitTime > 0)
							ChatAckSignal->WaitOne(waitTime, false);
						// 將收到的訊息包裏成SocketMessage物件
						if (ackMessage != nullptr)
						{
							responseMsg = gcnew SocketMessage(ackMessage, remoteIP, RemotePort);
						}					
					}
				}
			}
		}
		finally
		{
			Monitor::Exit(this);
		}

		return responseMsg;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 從Received Queue取出訊息
SocketMessage ^SocketConnection::PopDataInQueue()
{
	array<Byte> ^recvMsg;
	SocketMessage ^objSckMsg;

	try
	{
		// 取出資料
		Monitor::Enter(receivedQueue);
		try
		{
			recvMsg = safe_cast<array<Byte>^>(receivedQueue->Dequeue());
		}
		finally
		{
			Monitor::Exit(receivedQueue);
		}
		// 將收到的訊息包裏成SocketMessage物件
		objSckMsg = gcnew SocketMessage(recvMsg, remoteIP, remotePort);

		return objSckMsg;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete recvMsg;
	}
}


// 取得佇列數量
int SocketConnection::GetSendQueueCount()
{
	try
	{
		return sendQueue->Count;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 觸發訊息傳送執行緒
void SocketConnection::SetSignal(AutoResetEvent ^paramSignal)
{
	try
	{
		if (paramSignal != nullptr)
		{
			if ((!paramSignal->SafeWaitHandle->IsClosed) && (!paramSignal->SafeWaitHandle->IsInvalid))
			{
				paramSignal->Set();
			}
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除訊息傳送執行緒
void SocketConnection::ClearSignal(AutoResetEvent ^paramSignal)
{
	try
	{
		if (paramSignal != nullptr)
			paramSignal->Close();

		paramSignal = nullptr;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除所有資源
void SocketConnection::Close()
{
	try
	{	
		ReleaseResource();		
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// Destructor
SocketConnection::~SocketConnection()
{
	try
	{
		delete sckConnect;
		delete writerStream;
		delete sendThread;
		delete receivedQueue;
		delete sendQueue;
		delete SendSignal;
		delete SendPauseSignal;
		delete ChatAckSignal;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// -------------------------------------------- Private Functions -------------------------------------------//

// 取得遠端點傳來的訊息
void SocketConnection::ReceivedData(IAsyncResult ^paramResult)
{
	int readByteCount = 0;
	array<Byte> ^getMessage;
	StateObject ^stateObj;
	Socket ^recvSck;

	try
	{
		// 取得Socket相關資料與狀態
		stateObj = safe_cast<StateObject^>(paramResult->AsyncState);
		recvSck = stateObj->workSocket;
		readByteCount = recvSck->EndReceive(paramResult);

		// If there is received data
		if (readByteCount > 0)
		{
			// 由讀取緩衝區複製訊息內容
			getMessage = gcnew array<Byte>(readByteCount);
			Array::Copy(stateObj->buffer, getMessage, readByteCount);
			// 將訊息加入訊息接收緩衝佇列以待ReceiveMessageThread執行續處理
			Monitor::Enter(receivedQueue);
			try
			{
				receivedQueue->Enqueue(getMessage);
			}
			finally
			{
				Monitor::Exit(receivedQueue);
			}
			// Call processing function to pop up a received message
			if (isChat)
			{
				ackMessage = getMessage;
				// 觸發訊息接收訊號
				SetSignal(ChatAckSignal);
				isChat = false;
			}
			else
			{
				// 呼叫資料處理函式
				RecvInvoker(this);
			}
			stateObj->Clear();
			// 進行Socket訊息非同步讀取
			sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &SocketConnection::ReceivedData), stateObj);
		}
		else
		{
			OnDestroySocket(GetConnectionName(false));
		}
	}
	catch (ArgumentNullException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (ObjectDisposedException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (ArgumentOutOfRangeException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (SocketException ^ex)
	{
		tcpStatus = TcpStatus::Disconnected;
		OnDestroySocket(GetConnectionName(false));
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete getMessage;
	}
}


// 傳送本機訊息
void SocketConnection::SendDataInQueue()
{
	array<Byte> ^byteMsg;

	try
	{
		while (this->IsConnected)
		{
			if (sendQueue->Count > 0)
			{
				byteMsg = nullptr;
				Monitor::Enter(sendQueue);
				try
				{
					byteMsg = safe_cast<array<Byte>^>(sendQueue->Dequeue());
				}
				finally
				{
					Monitor::Exit(sendQueue);
				}
				// 將資料寫入資料串流傳送
				if (byteMsg != nullptr)
				{
					Monitor::Enter(this);
					try
					{
						writerStream->Write(byteMsg);
						writerStream->Flush();
					}
					finally
					{
						Monitor::Exit(this);
					}
				}
				// 避免Server端未能即時接收導致前後訊息相連
				if (sendPauseTime > 0)
					SendPauseSignal->WaitOne(sendPauseTime, false);
			}
			else
			{
				SendSignal->WaitOne();
			}
		}
	}
	catch (ObjectDisposedException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (IOException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete byteMsg;
	}
}


// 銷緩特定連線物件( 以 [IP:Port] 為關鍵值 )
void SocketConnection::OnDestroySocket(String^ paramName)
{
	try
	{
		DestroyEvent(paramName);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除所有在Received Queue裡的資料
void SocketConnection::ClearRecvQueue()
{
	try
	{
		Monitor::Enter(receivedQueue);
		try
		{
			receivedQueue->Clear();
		}
		finally
		{
			Monitor::Exit(receivedQueue);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除所有在Sending Queue裡的資料
void SocketConnection::ClearSendQueue()
{
	try
	{
		Monitor::Enter(sendQueue);
		try
		{
			sendQueue->Clear();
		}
		finally
		{
			Monitor::Exit(sendQueue);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 釋放所有資源
void SocketConnection::ReleaseResource()
{
	try
	{
		if (tcpStatus == TcpStatus::Connected)
			return;

		if (writerStream != nullptr)
		{
			writerStream->Close();
			writerStream = nullptr;
		}

		if (sckConnect != nullptr)
		{
			if (sckConnect->Connected)
			{
				sckConnect->Shutdown(SocketShutdown::Both);
				sckConnect->Close();
			}
			sckConnect = nullptr;
		}
		// 清除執行緒訊號
		ClearSignal(SendSignal);
		ClearSignal(SendPauseSignal);
		ClearSignal(ChatAckSignal);
		// 清除佇列資料
		ClearRecvQueue();
		ClearSendQueue();

		ackMessage = nullptr;

		if (sendThread != nullptr)
		{
			if (sendThread->IsAlive)
			{
				sendThread->Abort();
				sendThread = nullptr;
			}
		}
		// 變更連線狀態
		tcpStatus = TcpStatus::Disconnected;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


