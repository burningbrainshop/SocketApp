#pragma once

using namespace System::Diagnostics;

#include "stdafx.h"
#include "ISocket.h"
#include "AcceptArgument.h"
#include "ConnectChangedArgument.h"
#include "ReceivedArgument.h"
#include "SocketConnection.h"

public ref class TcpServer : ISocket
{
private:
	String ^listenerName;
	String ^listenerIP;
	String ^remoteIP;
	int listenerPort;
	int remotePort;
	int connectedCounter;
	int chatTimeout = 3;   // 3秒
	int sendPauseTime = 2;   // 2秒
	Encode encodeName = EncodeType;
	bool isConnStop = true;
	bool isConnected = false;
	bool isSingleConnection = false;
	Hashtable ^sckHash = gcnew Hashtable();

	TcpStatus tcpStatus = TcpStatus::None;	
	Socket ^sckListen;
	Socket ^sckClient;

	void AcceptCallback(IAsyncResult ^);
	void CleanSocket();
	void OnConnectChanged(ISocketConnection ^);
	void OnAccept(AcceptArgument ^);
	void OnReceived(ReceivedArgument ^);
	void OnDisconnected(ISocketConnection ^);
	void StopAccept();
	// Get Connection from Hashtable
	ISocketConnection ^GetConnection(String^);
	// Get Connection from Hashtable
	ISocketConnection ^GetConnection(String^, int);

public:
	// <summary> 連線狀態改變觸發事件(Connected) </summary>
	delegate void ConnectChangedEventHandler(Object ^src, ConnectChangedArgument ^args);

	// <summary> 連線請求發生時改變觸發事件 </summary>
	delegate void AcceptEventHandler(Object ^src, AcceptArgument ^args);

	// <summary> 收到訊息時觸發事件 </summary>
	delegate void ReceivedEventHandler(Object ^src, ReceivedArgument ^args);

	// <summary> 連線狀態改變觸發事件(Disconnected) </summary>
	delegate void DisconnectChangedEventHandler(Object ^src, ConnectChangedArgument ^args);

	event AcceptEventHandler ^AcceptEvent;
	event ConnectChangedEventHandler ^ConnectChangedEvent;
	event ReceivedEventHandler ^ReceivedEvent;
	event DisconnectChangedEventHandler ^DisconnectedEvent;


	// Constructor
	TcpServer();
	~TcpServer();

	// Listener Name
	property String ^Name
	{
		virtual String^ get() { return listenerName; }
		virtual void set(String^ paramName) { listenerName = paramName; }
	}

	// Local IP Value
	property String ^LocalIP
	{
		virtual String^ get() { return listenerIP; }
		virtual void set(String^ paramIP) { listenerIP = paramIP; }
	}

	// Local Port Value
	property int LocalPort
	{
		virtual int get() { return listenerPort; }
		virtual void set(int paramPort) { listenerPort = paramPort; }
	}

	// Remote IP Value
	property System::String ^RemoteIP
	{
		virtual String^ get() { return remoteIP; }
		virtual void set(String^ paramIP) { remoteIP = paramIP; }
	}

	// Remote Port Value
	property int RemotePort
	{
		virtual int get() { return remotePort; }
		virtual void set(int paramPort) { remotePort = paramPort; }
	}

	// Chat Message Timeout
	property int ChatTimeout
	{
		virtual int get() { return chatTimeout; }
		virtual void set(int paramTimeout) { chatTimeout = paramTimeout; }
	}

	// Encoding Value
	property Encode EncodeName
	{
		virtual Encode get() { return encodeName; }
		virtual void set(Encode paramCode) { encodeName = paramCode; }
	}

	// Connection Stop Status
	property bool IsStop
	{
		virtual bool get() { return IsStop; }
	}

	// Connection Status
	property bool IsConnected
	{
		virtual bool get()
		{
			if (tcpStatus == TcpStatus::Connected)
				isConnected = true;
			else
				isConnected = false;

			return isConnected;
		}
	}

	// Connection Count In Hashtable
	property int GetConnectionCount
	{
		int get();
	}

	// Single Connection
	property bool IsSingleConnection
	{
		bool get();
		void set(bool paramSingle);
	}

	// Start to Connect
	virtual void Start();

	// Stop the Connection
	virtual void Stop();

	// Close Socket
	virtual void Close();

	// Send Message with String value
	virtual void SendMessage(System::String ^paramIP, int paramPort, System::String ^paramMessage);

	// Send Message with Char value
	virtual void SendMessage(System::String ^paramIP, int paramPort, array<Byte> ^paramMessage);

	// Send Message with String value for Response
	virtual ISocketMessage ^ChatMessage(System::String ^paramIP, int paramPort, System::String ^paramMessage);

	// Send Message with Char value for Response
	virtual ISocketMessage ^ChatMessage(System::String ^paramIP, int paramPort, array<Byte> ^paramMessage);

	// Kill Connection in Hashtable
	void KillConnection(String^);

	// Received Data Callback Function
	void ReceivedCallback(ISocketConnection ^);

	// Accept Socket
	virtual Socket^ GetAcceptSocket();
};

