#pragma once

#include "ISocket.h"

using namespace System;

public ref class ReceivedArgument : EventArgs
{
private:
	ISocketMessage ^ sckMessage;

public:
	// 建構子
	ReceivedArgument(ISocketMessage ^paramSocket);

	property ISocketMessage^ SocketMessage
	{
		ISocketMessage^ get()
		{
			return sckMessage;
		}
	}
};
