#pragma once

using namespace System;

public interface class ISocketMessage
{
	// Char Value Message
	property array<Byte> ^Message;

	// Remote IP
	property String ^RemoteIP;

	// Remote Port
	property int RemotePort;

	// Received Message
	String ^GetMessage();

	// Copy Object
	ISocketMessage ^Copy();
};