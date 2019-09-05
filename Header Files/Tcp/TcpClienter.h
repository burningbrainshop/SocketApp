#pragma once

#include "stdafx.h"
#include "ISocket.h"
#include "AcceptArgument.h"
#include "ConnectChangedArgument.h"
#include "ReceivedArgument.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::IO;
using namespace System::Diagnostics;

public ref class TcpClienter : ISocket
{
private:
	String ^clientName;
	String ^localIP;
	String ^remoteIP;
	int localPort;
	int remotePort;
	int chatTimeout = 3;   // 3秒
	int sendPauseTime = 2;   // 2秒
	int bufferSize;
	Encode encodeName = EncodeType;
	bool isChat = false;
	bool isConnected = false;
	array<Byte> ^ackMessage;

	TcpStatus tcpStatus = TcpStatus::None;
	Socket ^sckConnect;
	BinaryWriter ^writerStream;
	Thread ^sendThread;
	Queue ^receivedQueue = gcnew Queue();
	Queue ^sendQueue = gcnew Queue();
	AutoResetEvent ^SendSignal = gcnew AutoResetEvent(false);
	AutoResetEvent ^SendPauseSignal = gcnew AutoResetEvent(false);
	AutoResetEvent ^ChatAckSignal = gcnew AutoResetEvent(false);

	void Received();
	void ConnectCallback(IAsyncResult ^);
	void ReceivedData(IAsyncResult ^);
	void SendDataInQueue();
	void OnConnectChanged();
	void OnReceived();
	void OnDisconnected();
	void SetSignal(AutoResetEvent ^);
	void ClearSignal(AutoResetEvent ^);
	void CleanSocket();
	void ClearRecvQueue();
	void ClearSendQueue();

public:
	// <summary> 連線狀態改變觸發事件(Connected) </summary>
	delegate void ConnectChangedEventHandler(Object ^src, ConnectChangedArgument ^args);

	// <summary> 連線請求發生時改變觸發事件 </summary>
	delegate void AcceptEventHandler(Object ^src, AcceptArgument ^args);

	// <summary> 收到訊息時觸發事件 </summary>
	delegate void ReceivedEventHandler(Object ^src, ReceivedArgument ^args);

	// <summary> 連線狀態改變觸發事件(Disconnected) </summary>
	delegate void DisconnectChangedEventHandler(Object ^src, ConnectChangedArgument ^args);

	event ConnectChangedEventHandler ^ConnectChangedEvent;
	event ReceivedEventHandler ^ReceivedEvent;
	event DisconnectChangedEventHandler ^DisconnectedEvent;

	// Constructor
	TcpClienter();
	~TcpClienter();

	// Listener Name
	property String ^Name
	{
		virtual String^ get() { return clientName; }
		virtual void set(String^ paramName) { clientName = paramName; }
	}

	// Local IP Value
	property String ^LocalIP
	{
		virtual String^ get() { return localIP; }
		virtual void set(String^ paramIP) { localIP = paramIP; }
	}

	// Local Port Value
	property int LocalPort
	{
		virtual int get() { return localPort; }
		virtual void set(int paramPort) { localPort = paramPort; }
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

	// Accept Socket
	virtual Socket^ GetAcceptSocket() sealed;
};
