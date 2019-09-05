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


	// <summary> 建構函數 </summary>
	// <param name="Client">Socket物件</param>
	AcceptArgument(ISocket ^paramSocket);
};
