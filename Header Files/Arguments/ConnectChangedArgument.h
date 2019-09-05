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
	// <summary> 建構函數 </summary>
	ConnectChangedArgument(String ^, int, String ^, int);
	// <summary> 建構函數 </summary>
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



