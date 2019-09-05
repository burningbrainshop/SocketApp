#include "stdafx.h"
#include "TcpClienter.h"
#include "PublicFuncs.h"
#include "StateObject.h"


// -------------------------------------------- Public Functions -------------------------------------------//

// �غc�l
TcpClienter::TcpClienter()
{
}


// �Ұ�Socket�s�u�{��
void TcpClienter::Start()
{
	IPEndPoint ^ep;

	try
	{
		if (tcpStatus == TcpStatus::Connected)
			return;

		sckConnect = gcnew Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
		// �j�w������}
		if (localIP->Trim()->Length > 0)
		{
			ep = gcnew IPEndPoint(IPAddress::Parse(localIP), localPort);
			sckConnect->Bind(ep);
			tcpStatus = TcpStatus::Bind;
		}
		// �s�u�ܻ��ݥD��
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


// ����s�u
void TcpClienter::Stop()
{
	tcpStatus = TcpStatus::Disconnected;
	CleanSocket();
}


// �����s�u
void TcpClienter::Close()
{
	Stop();
}


// �ǰe�T��(String�榡)
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


// �ǰe�T��(Byte�榡)
void TcpClienter::SendMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage)
{
	try
	{
		// �x�s�T���ܦ�C
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
			// Ĳ�o�T���o�e�����
			SetSignal(SendSignal);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �H��ͤ覡�ǰe�T��(String�榡)
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


// �H��ͤ覡�ǰe�T��(Byte�榡)
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
					// ���]������T��
					ChatAckSignal->Reset();
					isChat = true;
					// �ǰe��Ʀܻ����I, �õ��ݦ^���T��
					if (writerStream != nullptr)
					{
						writerStream->Write(paramMessage);
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


// ���o�s�uSocket
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

// �����I�s�u��^�I�禡
void TcpClienter::ConnectCallback(IAsyncResult ^paramResult)
{
	Socket ^sckTemp;
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// ���oSocket������ƻP���A
		sckTemp = safe_cast<Socket^>(paramResult->AsyncState);

		if (!sckTemp->Connected)
		{
			TcpStatus::Disconnected;
			return;
		}
		// �ܧ�s�u���A
		tcpStatus = TcpStatus::Connected;

		localIP = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Address->ToString();
		remoteIP = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Address->ToString();
		localPort = (safe_cast<IPEndPoint^>(sckConnect->LocalEndPoint))->Port;
		remotePort = (safe_cast<IPEndPoint^>(sckConnect->RemoteEndPoint))->Port;
		writerStream = gcnew BinaryWriter(gcnew NetworkStream(sckConnect, FileAccess::Write));

		// �إߨñҰʶǰe�T����C���T���������
		sendThread = gcnew Thread(gcnew ThreadStart(this, &TcpClienter::SendDataInQueue));
		sendThread->IsBackground = true;
		sendThread->Priority = ThreadPriority::Highest;
		sendThread->Start();

		// �]�w�s�u��socket����
		stateObj->workSocket = sckTemp;
		// �i��D�P�BŪ��
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &TcpClienter::ReceivedData), stateObj);
		
		// Ĳ�o�s�u�ƥ�
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


// �ҰʰT�������禡
void TcpClienter::Received()
{
	StateObject ^stateObj = gcnew StateObject;

	try
	{
		// �]�w�s�u��socket����
		stateObj->workSocket = sckConnect;
		// �i��D�P�BŪ��
		sckConnect->BeginReceive(stateObj->buffer, 0, stateObj->BUFFER_SIZE, SocketFlags::None, gcnew AsyncCallback(this, &TcpClienter::ReceivedData), stateObj);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �s�u�ƥ�
void TcpClienter::OnConnectChanged()
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(localIP, localPort, remoteIP, remotePort);
		// Ĳ�o�s�u�ƥ�
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


// ���oServer�ǨӪ��T��
void TcpClienter::ReceivedData(IAsyncResult ^paramResult)
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
				// Ĳ�o�T�������ƥ�
				OnReceived();
			}
			stateObj->Clear();
			// �i��Socket�T���D�P�BŪ��
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


// �T�������ƥ�
void TcpClienter::OnReceived()
{
	array<Byte> ^recvMsg;
	SocketMessage ^objSckMsg;
	ReceivedArgument ^args;

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
		// Pack up message as SocketMessage object
		objSckMsg = gcnew SocketMessage(recvMsg, remoteIP, remotePort);
		args = gcnew ReceivedArgument(objSckMsg);
		// Ĳ�o�T�������ƥ�
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


// Ĳ�o�s�u�����ƥ�
void TcpClienter::OnDisconnected()
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(localIP, localPort, remoteIP, remotePort);
		// Ĳ�o�s�u�ƥ�
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


// �ǰe�����T��
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


// Ĳ�o�T���ǰe�����
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


// �M���T���ǰe�����
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


// �M���Ҧ��귽
void TcpClienter::CleanSocket()
{
	try
	{
		// �M��IO�귽
		if (writerStream != nullptr)
		{
			writerStream->Close();
			writerStream = nullptr;
		}
		// �M��Socket�귽
		if (sckConnect != nullptr)
		{
			if (sckConnect->Connected)
			{
				sckConnect->Shutdown(SocketShutdown::Both);
				sckConnect->Close();
				sckConnect = nullptr;
			}
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
		tcpStatus = TcpStatus::None;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �M���Ҧ��bReceived Queue�̪����
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


// �M���Ҧ��bSending Queue�̪����
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