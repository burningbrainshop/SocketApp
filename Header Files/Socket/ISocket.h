#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Net;
using namespace System::Net::Sockets;

#include "SocketEnum.h"
#include "SocketMessage.h"

public interface class ISocket
{
	// Listener Name
	property String ^Name;

	// Local IP Value
	property String ^LocalIP;

	// Local Port Value
	property int LocalPort;

	// Remote IP Value
	property String ^RemoteIP;

	// Remote Port Value
	property int RemotePort;

	// Chat Message Timeout
	property int ChatTimeout;

	// Encoding Value
	property Encode EncodeName;

	// Connection Stop Status
	property bool IsStop
	{
		bool get();
	}

	// Connection Linking Status
	property bool IsConnected
	{
		bool get();
	}

	// Start to Connect
	void Start();

	// Stop the Connection
	void Stop();

	// Close Socket
	void Close();

	// Send Message with String value
	void SendMessage(String ^paramIP, int paramPort, String ^paramMessage);

	// Send Message with Char value
	void SendMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage);

	// Send Message with String value for Response
	ISocketMessage ^ChatMessage(String ^paramIP, int paramPort, String ^paramMessage);

	// Send Message with Char value for Response
	ISocketMessage ^ChatMessage(String ^paramIP, int paramPort, array<Byte> ^paramMessage);

	Socket^ GetAcceptSocket();
};


