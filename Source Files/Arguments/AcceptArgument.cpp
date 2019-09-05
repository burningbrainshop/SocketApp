#include "stdafx.h"
#include "AcceptArgument.h"



// 建構子
AcceptArgument::AcceptArgument(ISocket ^paramSocket)
{
	LocalIP = paramSocket->LocalIP;
	LocalPort = paramSocket->LocalPort;
	RemoteIP = paramSocket->RemoteIP;
	RemotePort = paramSocket->RemotePort;
}

