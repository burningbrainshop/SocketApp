#pragma once

#include "ISocket.h"

using namespace System;

public ref class ReceivedArgument : EventArgs
{
private:
	ISocketMessage ^ sckMessage;

public:
	// �غc���
	ReceivedArgument(ISocketMessage ^paramSocket);

	property ISocketMessage^ SocketMessage
	{
		ISocketMessage^ get()
		{
			return sckMessage;
		}
	}
};
