#include "stdafx.h"
#include "ConnectChangedArgument.h"


// 建構子
ConnectChangedArgument::ConnectChangedArgument(String ^paramLocalIP, int paramLocalPort, String ^paramRemoteIP, int paramRemotePort)
{
	localIP = paramLocalIP;
	localPort = paramLocalPort;
	remoteIP = paramRemoteIP;
	remotePort = paramRemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}


// 建構子
ConnectChangedArgument::ConnectChangedArgument(ISocketConnection ^paramSocket)
{
	localIP = paramSocket->ListenIP;
	localPort = paramSocket->ListenPort;
	remoteIP = paramSocket->RemoteIP;
	remotePort = paramSocket->RemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}
