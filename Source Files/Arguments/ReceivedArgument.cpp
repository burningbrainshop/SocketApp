#include "stdafx.h"
#include "ReceivedArgument.h"


// 建構子
ReceivedArgument::ReceivedArgument(ISocketMessage ^paramSocket)
{
	sckMessage = paramSocket;
}
