#pragma once

using namespace System;
using namespace System::Text;
using namespace System::Collections;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::IO;

#include "TcpServer.h"
#include "ISocketConnect.h"
#include "StateObject.h"

public ref class SocketConnection : ISocketConnection
{
	private:
	String ^localIP;
	String ^remoteIP;
	int localPort;
	int remotePort;
	int sendPauseTime;
	int bufferSize;
	double chatTimeout;   // 3秒
	bool isChat = false;
	bool isConnected = false;
	array<Byte> ^ackMessage;

	TcpStatus tcpStatus = TcpStatus::Connected;
	Socket ^sckConnect;
	BinaryWriter ^writerStream;
	Thread ^sendThread;
	Queue ^receivedQueue = gcnew Queue();
	Queue ^sendQueue = gcnew Queue();
	AutoResetEvent ^SendSignal = gcnew AutoResetEvent(false);
	AutoResetEvent ^SendPauseSignal = gcnew AutoResetEvent(false);
	AutoResetEvent ^ChatAckSignal = gcnew AutoResetEvent(false);

	void SendDataInQueue();
	void ReceivedData(IAsyncResult ^);
	void OnDestroySocket(String^);
	void SetSignal(AutoResetEvent ^);
	void ClearSignal(AutoResetEvent ^);
	void ClearRecvQueue();
	void ClearSendQueue();
	void ReleaseResource();

public:
	// 連線物件銷毀事件
	delegate void SocketDestroyEventHandler(String^);
	event SocketDestroyEventHandler ^DestroyEvent;
	// 訊息接收觸發函式
	delegate void MessageCallback(ISocketConnection ^);
	MessageCallback ^RecvInvoker;

	SocketConnection(ISocket ^);
	~SocketConnection();

	// 監聽端IP
	property String ^ListenIP
	{
		virtual String ^get() { return localIP; }
		virtual void set(String ^paramIP) { localIP = paramIP; }
	}
	// 監聽端Port
	property int ListenPort
	{
		virtual int get() { return localPort; }
		virtual void set(int paramPort) { localPort = paramPort; }
	}
	// 遠端連線IP
	property String ^RemoteIP
	{
		virtual String ^get() { return remoteIP; }
		virtual void set(String ^paramIP) { remoteIP = paramIP; }
	}
	// 遠端連線Port
	property int RemotePort
	{
		virtual int get() { return remotePort; }
		virtual void set(int paramPort) { remotePort = paramPort; }
	}
	// 交談等待時間(秒)
	property double ChatTimeOut
	{
		virtual double get() { return chatTimeout; }
		virtual void set(double paramTimeout) { chatTimeout = paramTimeout; }
	}
	// 訊息傳送延宕時間(秒)
	property int SendPauseTime
	{
		virtual int get() { return sendPauseTime; }
		virtual void set(int paramPausetime) { sendPauseTime = paramPausetime; }
	}
	// 緩街區大小
	property int BufferSize
	{
		virtual int get() { return bufferSize; }
		virtual void set(int paramSize) { bufferSize = paramSize; }
	}

	// Connection Status
	property bool IsConnected
	{
		bool get()
		{
			if (tcpStatus == TcpStatus::Connected)
				isConnected = true;
			else
				isConnected = false;

			return isConnected;
		}
	}

	void SendData(array<Byte> ^paramData);
	ISocketMessage ^ChatData(array<Byte> ^paramData);
	SocketMessage ^PopDataInQueue();
	virtual String ^GetConnectionName(bool);
	int GetSendQueueCount();
	void Close();
};
