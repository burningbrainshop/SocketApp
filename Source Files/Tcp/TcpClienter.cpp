#include "stdafx.h"
#include "TcpClienter.h"
#include "PublicFuncs.h"
#include "StateObject.h"


// -------------------------------------------- Public Functions -------------------------------------------//

// 建構子
TcpClienter::TcpClienter()
{
}


// 啟動Socket連線程序
void TcpClienter::Start()
{
	IPEndPoint ^ep;

	try
	{
		if (tcpStatus == TcpStatus::Connected)
			return;

		sckConnect = gcnew Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
		// 綁定本機位址
		if (localIP->Trim()->Length > 0)
		{
			ep = gcnew IPEndPoint(IPAddress::Parse(localIP), localPort);
			sckConnect->Bind(ep);
			tcpStatus = TcpStatus::Bind;
		}
		// 連線至遠端主機
		sckConnect->BeginConnect(gcnew IPEndPoint(IPAddress::Parse(remoteIP), remotePort), gcnew AsyncCallback(this, &TcpClienter::ConnectCallback), sckConnect);
	}
	catch (SocketException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
		OnDisconnected();
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete ep;
	}
}


// 停止連線
void TcpClienter::Stop()
{
	tcpStatus = TcpStatus::Disconnected;
	CleanSocket();
}


// 關閉連線
void TcpClienter::Close()
{
	Stop();
}


// 傳送訊息(String格式)
void TcpClienter::SendMessage(String ^paramIP, int paramPort, String ^paramMessage)
{
	try
	{
		LogRecord::GetInstance->WriteLog("[ Server ] Send Message  : " + paramMessage->ToString());
		SendMessage(paramIP, paramPort, PublicFuncs::ConvertStringToByte(encodeName, paramMessage));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 傳送訊息(Byte格式)
void TcpClienter::SendMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage)
{
	try
	{
		// 儲存訊息至佇列
		if (paramMessage != nullptr)
		{
			Monitor::Enter(sendQueue);
			try
			{
				sendQueue->Enqueue(paramMessage);
			}
			finally
			{
				Monitor::Exit(sendQueue);
			}
			// 觸發訊息發送執行緒
			SetSignal(SendSignal);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 以交談方式傳送訊息(String格式)
ISocketMessage ^TcpClienter::ChatMessage(String ^paramIP, int paramPort, String ^paramMessage)
{
	try
	{
		return ChatMessage(paramIP, paramPort, PublicFuncs::ConvertStringToByte(encodeName, paramMessage));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 以交談方式傳送訊息(Byte格式)
ISocketMessage ^TcpClienter::ChatMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage)
{
	ISocketMessage ^responseMsg = nullptr;
	DateTime stopTime;
	int waitTime;
	
	try
	{
		Monitor::Enter(this);
		try
		{
			if (tcpStatus == TcpStatus::Connected)
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
						writerStream->Write(paramMessage);
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


// 取得連線Socket
Socket^ TcpClienter::GetAcceptSocket()
{
	return nullptr;
}


// Destructor
TcpClienter::~TcpClienter()
{
	try
	{
		delete sckConnect;
		delete clientName;
		delete localIP;
		delete remoteIP;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}



// -------------------------------------------- Private Functions -------------------------------------------//

// 遠端點連線後回呼函式
void TcpClienter::ConnectCallback(IAsyncResult ^paramResult)
{
	Socket ^sckTemp;
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// 取得Socket相關資料與狀態
		sckTemp = safe_cast<Socket^>(paramResult->AsyncState);

		if (!sckTemp->Connected)
		{
			TcpStatus::Disconnected;
			return;
		}
		// 變更連線狀態
		tcpStatus = TcpStatus::Connected;

		localIP = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Address->ToString();
		remoteIP = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Address->ToString();
		localPort = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Port;
		remotePort = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Port;
		writerStream = gcnew BinaryWriter(gcnew NetworkStream(sckConnect, FileAccess::Write));

		// 建立並啟動傳送訊息佇列內訊息的執行緒
		sendThread = gcnew Thread(gcnew ThreadStart(this, &TcpClienter::SendDataInQueue));
		sendThread->IsBackground = true;
		sendThread->Priority = ThreadPriority::Highest;
		sendThread->Start();

		// 設定連線的socket物件
		stateObj->workSocket = sckTemp;
		// 進行非同步讀取
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &TcpClienter::ReceivedData), stateObj);
		
		// 觸發連線事件
		OnConnectChanged();	
	}
	catch (SocketException ^ex)
	{
		tcpStatus = TcpStatus::Disconnected;
		OnDisconnected();

		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 啟動訊息接收函式
void TcpClienter::Received()
{
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// 設定連線的socket物件
		stateObj->workSocket = sckConnect;
		// 進行非同步讀取
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &TcpClienter::ReceivedData), stateObj);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 連線事件
void TcpClienter::OnConnectChanged()
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(localIP, localPort, remoteIP, remotePort);
		// 觸發連線事件
		ConnectChangedEvent(this, args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete args;
	}
}


// 取得Server傳來的訊息
void TcpClienter::ReceivedData(IAsyncResult ^paramResult)
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
				// 觸發訊息接收事件
				OnReceived();
			}
			stateObj->Clear();
			// 進行Socket訊息非同步讀取
			sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &TcpClienter::ReceivedData), stateObj);
		}
		else
		{
			OnDisconnected();
		}
	}
	catch (SocketException ^ex)
	{
		OnDisconnected();
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
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
		delete getMessage;
	}
}


// 訊息接收事件
void TcpClienter::OnReceived()
{
	array<Byte> ^recvMsg;
	SocketMessage ^objSckMsg;
	ReceivedArgument ^args;

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
		// Pack up message as SocketMessage object
		objSckMsg = gcnew SocketMessage(recvMsg, remoteIP, remotePort);
		args = gcnew ReceivedArgument(objSckMsg);
		// 觸發訊息接收事件
		ReceivedEvent(this, args);
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


// 觸發連線結束事件
void TcpClienter::OnDisconnected()
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(localIP, localPort, remoteIP, remotePort);
		// 觸發連線事件
		DisconnectedEvent(this, args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete args;
	}
}


// 傳送本機訊息
void TcpClienter::SendDataInQueue()
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


// 觸發訊息傳送執行緒
void TcpClienter::SetSignal(AutoResetEvent ^paramSignal)
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
void TcpClienter::ClearSignal(AutoResetEvent ^paramSignal)
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
void TcpClienter::CleanSocket()
{
	try
	{
		// 清除IO資源
		if (writerStream != nullptr)
		{
			writerStream->Close();
			writerStream = nullptr;
		}
		// 清除Socket資源
		if (sckConnect != nullptr)
		{
			if (sckConnect->Connected)
			{
				sckConnect->Shutdown(SocketShutdown::Both);
				sckConnect->Close();
				sckConnect = nullptr;
			}
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
		tcpStatus = TcpStatus::None;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除所有在Received Queue裡的資料
void TcpClienter::ClearRecvQueue()
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
void TcpClienter::ClearSendQueue()
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