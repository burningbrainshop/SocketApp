#pragma once

#include "ISocket.h"

using namespace System;


public ref class AcceptArgument : EventArgs
{
public:
	String ^ LocalIP;
	String ^RemoteIP;
	int LocalPort;
	int RemotePort;
	bool IsAccept = true;


	// 建構子
	AcceptArgument(ISocket ^paramSocket);
};
