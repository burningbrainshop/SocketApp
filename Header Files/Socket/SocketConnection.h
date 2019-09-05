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
	double chatTimeout;   // 3��
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
	// �s�u����P���ƥ�
	delegate void SocketDestroyEventHandler(String^);
	event SocketDestroyEventHandler ^DestroyEvent;
	// �T������Ĳ�o�禡
	delegate void MessageCallback(ISocketConnection ^);
	MessageCallback ^RecvInvoker;

	SocketConnection(ISocket ^);
	~SocketConnection();

	// ��ť��IP
	property String ^ListenIP
	{
		virtual String ^get() { return localIP; }
		virtual void set(String ^paramIP) { localIP = paramIP; }
	}
	// ��ť��Port
	property int ListenPort
	{
		virtual int get() { return localPort; }
		virtual void set(int paramPort) { localPort = paramPort; }
	}
	// ���ݳs�uIP
	property String ^RemoteIP
	{
		virtual String ^get() { return remoteIP; }
		virtual void set(String ^paramIP) { remoteIP = paramIP; }
	}
	// ���ݳs�uPort
	property int RemotePort
	{
		virtual int get() { return remotePort; }
		virtual void set(int paramPort) { remotePort = paramPort; }
	}
	// ��͵��ݮɶ�(��)
	property double ChatTimeOut
	{
		virtual double get() { return chatTimeout; }
		virtual void set(double paramTimeout) { chatTimeout = paramTimeout; }
	}
	// �T���ǰe���X�ɶ�(��)
	property int SendPauseTime
	{
		virtual int get() { return sendPauseTime; }
		virtual void set(int paramPausetime) { sendPauseTime = paramPausetime; }
	}
	// �w��Ϥj�p
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
