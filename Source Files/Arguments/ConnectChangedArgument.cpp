#include "stdafx.h"
#include "ConnectChangedArgument.h"


// <summary> 建構函數 </summary>
ConnectChangedArgument::ConnectChangedArgument(String ^paramLocalIP, int paramLocalPort, String ^paramRemoteIP, int paramRemotePort)
{
	localIP = paramLocalIP;
	localPort = paramLocalPort;
	remoteIP = paramRemoteIP;
	remotePort = paramRemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}


// <summary> 建構函數 </summary>
// <param name="paramSocket">Socket物件</param>
ConnectChangedArgument::ConnectChangedArgument(ISocketConnection ^paramSocket)
{
	localIP = paramSocket->ListenIP;
	localPort = paramSocket->ListenPort;
	remoteIP = paramSocket->RemoteIP;
	remotePort = paramSocket->RemotePort;
	connectedTime = DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss");
}
