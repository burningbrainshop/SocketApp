#include "stdafx.h"
#include "TcpServer.h"
#include "PublicFuncs.h"


// -------------------------------------------- Property -------------------------------------------//

// 取得連線數量
int TcpServer::GetConnectionCount::get()
{
	return connectedCounter;
}


// 是否為單一連線
bool TcpServer::IsSingleConnection::get()
{
	return isSingleConnection;
}

void TcpServer::IsSingleConnection::set(bool paramSingle)
{
	isSingleConnection = paramSingle;
}


// -------------------------------------------- Public Functions -------------------------------------------//

// 建構子
TcpServer::TcpServer()
{
}


// 啟動監聽程序
void TcpServer::Start()
{
	IPEndPoint ^ep;

	try
	{
		// 設定參數
		sckListen = gcnew Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
		sckListen->SetSocketOption(SocketOptionLevel::Socket, SocketOptionName::DontLinger, 0);
		ep = gcnew IPEndPoint(IPAddress::Parse(listenerIP), listenerPort);
		// 綁定本機位址
		sckListen->Bind(ep);
		// 開始監聽
		sckListen->Listen(30);
		// 變更連線狀態
		tcpStatus = TcpStatus::Listen;
		// 啟動非同步接收連線程序
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


// 停止連線
void TcpServer::Stop()
{
	tcpStatus = TcpStatus::Disconnected;
	StopAccept();
}


// 關閉連線
void TcpServer::Close()
{
	Stop();
}


// 傳送訊息(String格式)
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


// 傳送訊息(Byte格式)
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


// 以交談方式傳送訊息(String格式)
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


// 以交談方式傳送訊息(Byte格式)
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


// 取得特定連線物件( 以 [IP:Port] 為關鍵值 )
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


// 取得特定連線物件( 輸入 [IP] 和 [Port] )
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


// 銷緩儲存於Hashtable的特定連線
void TcpServer::KillConnection(String^ paramName)
{
	SocketConnection ^sckConn;

	try
	{
		// 取得指定連線
		sckConn = safe_cast<SocketConnection^>(GetConnection(paramName));
		// 關閉連線
		if (sckConn != nullptr)
			sckConn->Close();
		// 觸發斷線事件
		OnDisconnected(safe_cast<ISocketConnection^>(sckConn));
		// 從Hashtable裡刪除指定連線
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


// 取得連線物件
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

// 處理接受連線物件
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
			// 觸發Socket接收事件
			OnAccept(acceptArgs);
			// 設定連線物件	
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
					// 取得連線名稱
					tempConnName = sckConnection->GetConnectionName(isSingleConnection);
					if (sckHash->Contains(tempConnName))
					{
						safe_cast<SocketConnection^>(sckHash[tempConnName])->Close();
						sckHash->Remove(tempConnName);
					}
					// 儲存連線物件
					sckHash->Add(tempConnName, sckConnection);
				}
				finally
				{
					Monitor::Exit(sckHash);
				}
				// 觸發Socket連線事件
				OnConnectChanged(sckConnection);		
			}
			else
			{
				sckClient->Shutdown(SocketShutdown::Both);
				sckClient->Close();
			}
		}
		// 啟動非同步接收連線程序
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


// 資料收取回呼函式 : 處理已收到的資料
void TcpServer::ReceivedCallback(ISocketConnection ^paramConn)
{
	ISocketMessage ^objSckMsg;
	ReceivedArgument^ args;

	try
	{
		objSckMsg = safe_cast<SocketConnection^>(paramConn)->PopDataInQueue();
		args = gcnew ReceivedArgument(objSckMsg);
		// 觸發資料接收事件
		OnReceived(args);
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 觸發連線事件
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


// 觸發連線接收事件
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


// 觸發資料收取事件
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


// 觸發連線斷線事件
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


// 關閉監聽
void TcpServer::StopAccept()
{
	try
	{
		if (tcpStatus == TcpStatus::None)
			return;
		// 清除所有連線
		if (sckHash->Count > 0)
		{
			for each (DictionaryEntry item in sckHash)
				safe_cast<SocketConnection^>(item.Value)->Close();

			sckHash->Clear();
		}
		// 關閉監聽Socket
		CleanSocket();
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}


// 清除監聽連線
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