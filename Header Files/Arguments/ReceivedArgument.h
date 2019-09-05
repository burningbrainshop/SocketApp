#pragma once

#include "ISocket.h"

using namespace System;

public ref class ReceivedArgument : EventArgs
{
private:
	ISocketMessage ^ sckMessage;

public:
	// <summary> «Øºc¨ç¼Æ </summary>
	ReceivedArgument(ISocketMessage ^paramSocket);

	property ISocketMessage^ SocketMessage
	{
		ISocketMessage^ get()
		{
			return sckMessage;
		}
	}
};
