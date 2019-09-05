#include "stdafx.h"
#include "PublicFuncs.h"


// Convert Byte Data to String
String ^PublicFuncs::ReturnMessageByEncode(Encode paramCode, array<Byte>^ sckMessage)
{
	String^ tempMessage = nullptr;

	try
	{
		switch (paramCode)
		{
		case Encode::ASCII:
			tempMessage = Encoding::ASCII->GetString(sckMessage);
			break;
		case Encode::Unicode:
			tempMessage = Encoding::Unicode->GetString(sckMessage);
			break;
		case Encode::UTF8:
			tempMessage = Encoding::UTF8->GetString(sckMessage);
			break;
		case Encode::Default:
			tempMessage = Encoding::Default->GetString(sckMessage);
			break;
		default:
			break;
		}

		return tempMessage;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete tempMessage;
	}
}


// Convert String to Byte Data
array<Byte> ^PublicFuncs::ConvertStringToByte(Encode paramCode, String^ sckMessage)
{
	array<Byte>^ tempMessage = nullptr;

	try
	{
		switch (paramCode)
		{
		case Encode::ASCII:
			tempMessage = Encoding::ASCII->GetBytes(sckMessage);
			break;
		case Encode::Unicode:
			tempMessage = Encoding::Unicode->GetBytes(sckMessage);
			break;
		case Encode::UTF8:
			tempMessage = Encoding::UTF8->GetBytes(sckMessage);
			break;
		case Encode::Default:
			tempMessage = Encoding::Default->GetBytes(sckMessage);
			break;
		default:
			break;
		}

		return tempMessage;
	}
	catch (Exception ^ex)
	{
		LogRecord::GetInstance->WirteException(ex, gcnew StackTrace(true));
	}
	finally
	{
		delete tempMessage;
	}
}
