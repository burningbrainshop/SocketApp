#include "stdafx.h"
#include "SocketMessage.h"
#include "PublicFuncs.h"



// Char Value Message
array<Byte> ^SocketMessage::Message::get()
{
	return sckMessage;
}

void SocketMessage::Message::set(array<Byte> ^paramMessage)
{
	sckMessage = paramMessage;
}


// Remote IP
String ^SocketMessage::RemoteIP::get()
{
	return remoteIP;
}

void SocketMessage::RemoteIP::set(String ^paramIP)
{
	remoteIP = paramIP;
}


// Remote Port
int SocketMessage::RemotePort::get()
{
	return remotePort;
}

void SocketMessage::RemotePort::set(int paramPort)
{
	remotePort = paramPort;
}


// Encode Name
Encode SocketMessage::EncodeName::get()
{
	return encodeName;
}

void SocketMessage::EncodeName::set(Encode paramCode)
{
	encodeName = paramCode;
}


// Constructor
SocketMessage::SocketMessage(array<Byte> ^paramMessage, String ^paramIP, int paramPort) :
	sckMessage(paramMessage), remoteIP(paramIP), remotePort(paramPort)
{
}


// Constructor
SocketMessage::SocketMessage(SocketMessage ^SckMsg)
{
	this->sckMessage = SckMsg->Message;
	this->remoteIP = SckMsg->RemoteIP;
	this->remotePort = SckMsg->RemotePort;
}


// Received Message
String ^SocketMessage::GetMessage()
{
	String ^temp;

	try
	{
		if (sckMessage == nullptr)
			temp = nullptr;
		else
			temp = PublicFuncs::ReturnMessageByEncode(encodeName, sckMessage);

		return temp;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete temp;
	}
}


// Copy Object
ISocketMessage ^SocketMessage::Copy()
{
	SocketMessage ^SckMsgObj = nullptr;

	try
	{
		SckMsgObj = gcnew SocketMessage(this);
		return SckMsgObj;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete SckMsgObj;
	}
}


// Destructor
SocketMessage::~SocketMessage()
{
	try
	{
		delete sckMessage;
		delete remoteIP;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}