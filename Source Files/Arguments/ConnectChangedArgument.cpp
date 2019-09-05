#include "stdafx.h"
#include "ConnectChangedArgument.h"


// <summary> �غc��� </summary>
ConnectChangedArgument::ConnectChangedArgument(String ^paramLocalIP, int paramLocalPort, String ^paramRemoteIP, int paramRemotePort)
{
	localIP = paramLocalIP;
	localPort = paramLocalPort;
	remoteIP = paramRemoteIP;
	remotePort = paramRemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}


// <summary> �غc��� </summary>
// <param name="paramSocket">Socket����</param>
ConnectChangedArgument::ConnectChangedArgument(ISocketConnection ^paramSocket)
{
	localIP = paramSocket->ListenIP;
	localPort = paramSocket->ListenPort;
	remoteIP = paramSocket->RemoteIP;
	remotePort = paramSocket->RemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}
