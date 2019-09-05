#include "stdafx.h"
#include "StateObject.h"


void StateObject::Clear()
{
	try
	{
		if (buffer != nullptr)
		{
			Array::Clear(buffer, 0, BUFFER_SIZE);
		}		
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
}