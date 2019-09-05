#pragma once

using namespace System;

#include "ISocketConnect.h"


public ref class ConnectChangedArgument : EventArgs
{
private:
	String ^localIP;
	String ^remoteIP;
	int localPort;
	int remotePort;
	String ^connectedTime;

public:
	// 建構子
	ConnectChangedArgument(String ^, int, String ^, int);
	// 建構子
	ConnectChangedArgument(ISocketConnection ^paramSocket);

	property String ^ LocalIP
	{
		String ^get() { return localIP; }
	}

	property String ^RemoteIP
	{
		String ^get() { return remoteIP; }
	}

	property int LocalPort
	{
		int get() { return localPort; }
	}

	property int RemotePort
	{
		int get() { return remotePort; }
	}
	
	property String ^ConnectedTime
	{
		String ^get() { return connectedTime; }
	}
};



