#pragma once

using namespace System;
using namespace System::Text;

public interface class ISocketConnection
{
	property String ^ListenIP;

	property int ListenPort;

	property String ^RemoteIP;

	property int RemotePort;

	property double ChatTimeOut;

	property int SendPauseTime;

	property int BufferSize;

	String ^GetConnectionName(bool);
};
