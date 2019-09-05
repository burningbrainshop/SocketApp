#include "stdafx.h"
#include "AcceptArgument.h"



// <summary> 建構函數 </summary>
// <param name="paramSocket">Socket物件</param>
AcceptArgument::AcceptArgument(ISocket ^paramSocket)
{
	LocalIP = paramSocket->LocalIP;
	LocalPort = paramSocket->LocalPort;
	RemoteIP = paramSocket->RemoteIP;
	RemotePort = paramSocket->RemotePort;
}

