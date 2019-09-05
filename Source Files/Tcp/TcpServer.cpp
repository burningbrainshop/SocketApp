#include "stdafx.h"
#include "TcpServer.h"
#include "PublicFuncs.h"


// -------------------------------------------- Property -------------------------------------------//

// ���o�s�u�ƶq
int TcpServer::GetConnectionCount::get()
{
	return connectedCounter;
}


// �O�_����@�s�u
bool TcpServer::IsSingleConnection::get()
{
	return isSingleConnection;
}

void TcpServer::IsSingleConnection::set(bool paramSingle)
{
	isSingleConnection = paramSingle;
}


// -------------------------------------------- Public Functions -------------------------------------------//

// �غc�l
TcpServer::TcpServer()
{
}


// �Ұʺ�ť�{��
void TcpServer::Start()
{
	IPEndPoint ^ep;

	try
	{
		// �]�w�Ѽ�
		sckListen = gcnew Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
		sckListen->SetSocketOption(SocketOptionLevel::Socket, SocketOptionName::DontLinger, 0);
		ep = gcnew IPEndPoint(IPAddress::Parse(listenerIP), listenerPort);
		// �j�w������}
		sckListen->Bind(ep);
		// �}�l��ť
		sckListen->Listen(30);
		// �ܧ�s�u���A
		tcpStatus = TcpStatus::Listen;
		// �ҰʫD�P�B�����s�u�{��
		sckListen->BeginAccept(gcnew AsyncCallback(this, &TcpServer::AcceptCallback), sckListen);
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
void TcpServer::Stop()
{
	tcpStatus = TcpStatus::Disconnected;
	StopAccept();
}


// �����s�u
void TcpServer::Close()
{
	Stop();
}


// �ǰe�T��(String�榡)
void TcpServer::SendMessage(String ^paramIP, int paramPort, String ^paramMessage)
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
void TcpServer::SendMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage)
{
	SocketConnection ^sckConn;

	try
	{
		for each(DictionaryEntry item in sckHash)
		{
			sckConn = nullptr;
			sckConn = safe_cast<SocketConnection^>(item.Value);
			if (sckConn->RemoteIP == paramIP && sckConn->RemotePort == paramPort)
				break;
		}

		if (sckConn != nullptr)
			sckConn->SendData(paramMessage);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �H��ͤ覡�ǰe�T��(String�榡)
ISocketMessage ^TcpServer::ChatMessage(String ^paramIP, int paramPort, String ^paramMessage)
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
ISocketMessage ^TcpServer::ChatMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage)
{
	ISocketMessage ^objSckMsg = nullptr;
	SocketConnection ^sckConn;

	try
	{
		sckConn = safe_cast<SocketConnection^>(GetConnection(paramIP, paramPort));
		if (sckConn != nullptr)
			objSckMsg = sckConn->ChatData(paramMessage);

		return objSckMsg;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete objSckMsg;
		delete sckConn;
	}
}


// ���o�S�w�s�u����( �H [IP:Port] ������� )
ISocketConnection ^TcpServer::GetConnection(String^ paramName)
{
	SocketConnection ^sckConn = nullptr;

	try
	{
		if (sckHash->Contains(paramName))
			sckConn = safe_cast<SocketConnection^>(sckHash[paramName]);

		return sckConn;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete sckConn;
	}
}


// ���o�S�w�s�u����( ��J [IP] �M [Port] )
ISocketConnection ^TcpServer::GetConnection(String^ paramIP, int paramPort)
{
	SocketConnection ^sckConn = nullptr;

	try
	{
		for each(DictionaryEntry item in sckHash)
		{
			sckConn = nullptr;
			sckConn = safe_cast<SocketConnection^>(item.Value);
			if (sckConn->RemoteIP == paramIP && sckConn->RemotePort == paramPort)
				break;
		}

		return sckConn;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete sckConn;
	}
}


// �P�w�x�s��Hashtable���S�w�s�u
void TcpServer::KillConnection(String^ paramName)
{
	SocketConnection ^sckConn;

	try
	{
		// ���o���w�s�u
		sckConn = safe_cast<SocketConnection^>(GetConnection(paramName));
		// �����s�u
		if (sckConn != nullptr)
			sckConn->Close();
		// Ĳ�o�_�u�ƥ�
		OnDisconnected(safe_cast<ISocketConnection^>(sckConn));
		// �qHashtable�̧R�����w�s�u
		Monitor::Enter(sckHash);
		try
		{
			sckHash->Remove(paramName);
		}
		finally
		{
			Monitor::Exit(sckHash);
		}
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete sckConn;
	}
}


// ���o�s�u����
Socket^ TcpServer::GetAcceptSocket()
{
	return sckClient;
}


// Destructor
TcpServer::~TcpServer()
{
	try
	{
		delete sckHash;
		delete sckListen;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// -------------------------------------------- Private Functions -------------------------------------------//

// �B�z�����s�u����
void TcpServer::AcceptCallback(IAsyncResult ^paramAr)
{
	SocketConnection ^sckConnection;
	AcceptArgument ^acceptArgs;
	String^ tempConnName;
	Socket ^sckListenTemp;
	
	try
	{
		if (sckListen != nullptr && sckListen->Handle.ToInt32() != -1)
		{
			// Get the connection socket
			sckListenTemp = dynamic_cast<Socket^>(paramAr->AsyncState);
			// End the accept operation
			sckClient = sckListenTemp->EndAccept(paramAr);
		}
					
		if (sckClient != nullptr)
		{
			acceptArgs = gcnew AcceptArgument(safe_cast<ISocket^>(this));
			// Ĳ�oSocket�����ƥ�
			OnAccept(acceptArgs);
			// �]�w�s�u����	
			sckConnection = gcnew SocketConnection(this);
			sckConnection->SendPauseTime = sendPauseTime;
			sckConnection->BufferSize = BuffSize;
			sckConnection->ChatTimeOut = chatTimeout;
			sckConnection->DestroyEvent += gcnew SocketConnection::SocketDestroyEventHandler(this, &TcpServer::KillConnection);

			if (acceptArgs->IsAccept)
			{
				Monitor::Enter(sckHash);
				try
				{
					// ���o�s�u�W��
					tempConnName = sckConnection->GetConnectionName(isSingleConnection);
					if (sckHash->Contains(tempConnName))
					{
						safe_cast<SocketConnection^>(sckHash[tempConnName])->Close();
						sckHash->Remove(tempConnName);
					}
					// �x�s�s�u����
					sckHash->Add(tempConnName, sckConnection);
				}
				finally
				{
					Monitor::Exit(sckHash);
				}
				// Ĳ�oSocket�s�u�ƥ�
				OnConnectChanged(sckConnection);		
			}
			else
			{
				sckClient->Shutdown(SocketShutdown::Both);
				sckClient->Close();
			}
		}
		// �ҰʫD�P�B�����s�u�{��
		sckListen->BeginAccept(gcnew AsyncCallback(this, &TcpServer::AcceptCallback), sckListen);
	}
	catch (InvalidCastException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (InvalidOperationException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (SocketException ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// ��Ʀ����^�I�禡 : �B�z�w���쪺���
void TcpServer::ReceivedCallback(ISocketConnection ^paramConn)
{
	ISocketMessage ^objSckMsg;
	ReceivedArgument^ args;

	try
	{
		objSckMsg = safe_cast<SocketConnection^>(paramConn)->PopDataInQueue();
		args = gcnew ReceivedArgument(objSckMsg);
		// Ĳ�o��Ʊ����ƥ�
		OnReceived(args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// Ĳ�o�s�u�ƥ�
void TcpServer::OnConnectChanged(ISocketConnection ^sckConn)
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(sckConn);
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


// Ĳ�o�s�u�����ƥ�
void TcpServer::OnAccept(AcceptArgument ^args)
{
	try
	{
		AcceptEvent(this, args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// Ĳ�o��Ʀ����ƥ�
void TcpServer::OnReceived(ReceivedArgument ^args)
{
	try
	{
		ReceivedEvent(this, args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// Ĳ�o�s�u�_�u�ƥ�
void TcpServer::OnDisconnected(ISocketConnection ^sckConn)
{
	ConnectChangedArgument^ args;

	try
	{
		args = gcnew ConnectChangedArgument(sckConn);
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


// ������ť
void TcpServer::StopAccept()
{
	try
	{
		if (tcpStatus == TcpStatus::None)
			return;
		// �M���Ҧ��s�u
		if (sckHash->Count > 0)
		{
			for each (DictionaryEntry item in sckHash)
				safe_cast<SocketConnection^>(item.Value)->Close();

			sckHash->Clear();
		}
		// ������ťSocket
		CleanSocket();
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// �M����ť�s�u
void TcpServer::CleanSocket()
{
	try
	{
		if (sckListen != nullptr)
		{
			if (sckListen->Handle.ToInt32() != -1)
				sckListen->Close();
		}

		sckListen = nullptr;
		tcpStatus = TcpStatus::None;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}