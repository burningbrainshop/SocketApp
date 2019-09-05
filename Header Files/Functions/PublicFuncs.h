#pragma once

#include "SocketEnum.h"

using namespace System;
using namespace System::Text;
using namespace System::Diagnostics;

ref class PublicFuncs
{
public:
	static String ^ReturnMessageByEncode(Encode, array<Byte>^);
	static array<Byte> ^ConvertStringToByte(Encode, String^);
};
