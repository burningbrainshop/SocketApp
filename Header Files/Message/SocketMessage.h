#pragma once

#include "ISocketMessage.h"
#include "SocketEnum.h"

#define BuffSize 8192
#define EncodeType Encode::UTF8

using namespace System;
using namespace System::Text;
using namespace System::Diagnostics;


ref class SocketMessage : ISocketMessage
{
private:
	array<Byte>^ sckMessage = gcnew array<Byte>(BuffSize);
	String ^remoteIP;
	int remotePort;
	Encode encodeName = EncodeType;

public:
	SocketMessage(array<Byte> ^paramMessage, String ^paramIP, int paramPort);
	SocketMessage(SocketMessage ^SckMsg);
	~SocketMessage();

	// Char Value Message
	virtual property array<Byte> ^Message
	{
		array<Byte> ^get();
		void set(array<Byte> ^);
	}

	// Remote IP
	virtual property String ^RemoteIP
	{
		String ^get();
		void set(String ^);
	}

	// Remote Port
	virtual property int RemotePort
	{
		int get();
		void set(int);
	}

	property Encode EncodeName
	{
		Encode get();
		void set(Encode);
	}

	// Received Message
	virtual String ^GetMessage();

	// Copy Object
	virtual ISocketMessage ^Copy();
};

