#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::IO;
using namespace System::Diagnostics;

public ref class StateObject
{
public:
	literal int BUFFER_SIZE = 8196;
	Socket^ workSocket;
	array<Byte>^ buffer;
	StringBuilder^ sb;

	StateObject() : workSocket(nullptr)
	{
		buffer = gcnew array<Byte>(BUFFER_SIZE);
		sb = gcnew StringBuilder;
	}

	void Clear();
};