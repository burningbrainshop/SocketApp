#pragma once

public enum class Encode
{
	ASCII,
	Unicode,
	UTF8,
	Default
};


public enum class TcpStatus
{
	Bind,
	Listen,
	Accept,
	Connected,
	Disconnected,
	Received,
	None
};
