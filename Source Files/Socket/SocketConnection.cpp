#include "stdafx.h"
#include "SocketConnection.h"


// �զX�s�u�W�� : [IP] / [IP:Port] 
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

// �غc�l
SocketConnection::SocketConnection(ISocket ^paramServer)
{
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// ���o�s�u
		sckConnect = paramServer->GetAcceptSocket();
		RecvInvoker = gcnew MessageCallback(safe_cast<TcpServer^>(paramServer), &TcpServer::ReceivedCallback);
		// �]�w�Ѽ�
		localIP = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Address->ToString();
		remoteIP = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Address->ToString();
		localPort = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Port;
		remotePort = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Port;
		isConnected = true;

		writerStream = gcnew BinaryWriter(gcnew NetworkStream(sckConnect, FileAccess::Write));
		stateObj->workSocket = sckConnect;

		// �إߨñҰʶǰe�T����C���T���������
		sendThread = gcnew Thread(gcnew ThreadStart(this, &SocketConnection::SendDataInQueue));
		sendThread->IsBackground = true;
		sendThread->Priority = ThreadPriority::Highest;
		sendThread->Start();

		// �i��D�P�BŪ��
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &SocketConnection::ReceivedData), stateObj);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �ǰe�T�� : ����J��C���ݶǰe
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


// �H�P�B�覡(Blocking)�o�e�T���A�|���ݰT���^��
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
					// ���]������T��
					ChatAckSignal->Reset();
					isChat = true;
					// �ǰe��Ʀܻ����I, �õ��ݦ^���T��
					if (writerStream != nullptr)
					{
						writerStream->Write(paramData);
						writerStream->Flush();
						waitTime = safe_cast<int>((safe_cast<TimeSpan^>(safe_cast<DateTime^>(stopTime)->Subtract(DateTime::Now)))->TotalMilliseconds);
						// ���ݰT���^�ǰT��
						if (waitTime > 0)
							ChatAckSignal->WaitOne(waitTime, false);
						// �N���쪺�T���]�ئ�SocketMessage����
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


// �qReceived Queue���X�T��
SocketMessage ^SocketConnection::PopDataInQueue()
{
	array<Byte> ^recvMsg;
	SocketMessage ^objSckMsg;

	try
	{
		// ���X���
		Monitor::Enter(receivedQueue);
		try
		{
			recvMsg = safe_cast<array<Byte>^>(receivedQueue->Dequeue());
		}
		finally
		{
			Monitor::Exit(receivedQueue);
		}
		// �N���쪺�T���]�ئ�SocketMessage����
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


// ���o��C�ƶq
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


// Ĳ�o�T���ǰe�����
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


// �M���T���ǰe�����
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


// �M���Ҧ��귽
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

// ���o�����I�ǨӪ��T��
void SocketConnection::ReceivedData(IAsyncResult ^paramResult)
{
	int readByteCount = 0;
	array<Byte> ^getMessage;
	StateObject ^stateObj;
	Socket ^recvSck;

	try
	{
		// ���oSocket������ƻP���A
		stateObj = safe_cast<StateObject^>(paramResult->AsyncState);
		recvSck = stateObj->workSocket;
		readByteCount = recvSck->EndReceive(paramResult);

		// If there is received data
		if (readByteCount > 0)
		{
			// ��Ū���w�İϽƻs�T�����e
			getMessage = gcnew array<Byte>(readByteCount);
			Array::Copy(stateObj->buffer, getMessage, readByteCount);
			// �N�T���[�J�T�������w�Ħ�C�H��ReceiveMessageThread������B�z
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
				// Ĳ�o�T�������T��
				SetSignal(ChatAckSignal);
				isChat = false;
			}
			else
			{
				// �I�s��ƳB�z�禡
				RecvInvoker(this);
			}
			stateObj->Clear();
			// �i��Socket�T���D�P�BŪ��
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


// �ǰe�����T��
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
				// �N��Ƽg�J��Ʀ�y�ǰe
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
				// �קKServer�ݥ���Y�ɱ����ɭP�e��T���۳s
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


// �P�w�S�w�s�u����( �H [IP:Port] ������� )
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


// �M���Ҧ��bReceived Queue�̪����
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


// �M���Ҧ��bSending Queue�̪����
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


// ����Ҧ��귽
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
		// �M��������T��
		ClearSignal(SendSignal);
		ClearSignal(SendPauseSignal);
		ClearSignal(ChatAckSignal);
		// �M����C���
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
		// �ܧ�s�u���A
		tcpStatus = TcpStatus::Disconnected;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


