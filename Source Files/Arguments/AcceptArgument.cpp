#include "stdafx.h"
#include "AcceptArgument.h"



// <summary> �غc��� </summary>
// <param name="paramSocket">Socket����</param>
AcceptArgument::AcceptArgument(ISocket ^paramSocket)
{
	LocalIP = paramSocket->LocalIP;
	LocalPort = paramSocket->LocalPort;
	RemoteIP = paramSocket->RemoteIP;
	RemotePort = paramSocket->RemotePort;
}

