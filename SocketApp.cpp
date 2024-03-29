#include "stdafx.h"
#include "TcpServer.h"
//#include "TcpClienter.h"

using namespace System;

int clientPort;

void TcpServer_ConnectAccept(Object ^src, AcceptArgument ^args)
{
	try
	{
		Console::WriteLine("[ Server ] System : A client had been accepted by Server.");
		Console::WriteLine("[ Server ] Info : LocalIP => " + args->LocalIP);
		Console::WriteLine("[ Server ] Info : LocalPort => " + args->LocalPort);
		Console::WriteLine("[ Server ] Info : RemoteIP => " + args->RemoteIP);
		Console::WriteLine("[ Server ] Info : RemotePort => " + args->RemotePort);
	}
	catch (Exception ^ex)
	{
		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
	}
}

void TcpServer_ConnectChanged(Object ^src, ConnectChangedArgument ^args)
{
	try
	{
		Console::WriteLine("[ Server ] System : A client had connected to Server.");
		Console::WriteLine("[ Server ] Info : LocalIP => " + args->LocalIP);
		Console::WriteLine("[ Server ] Info : LocalPort => " + args->LocalPort);
		Console::WriteLine("[ Server ] Info : RemoteIP => " + args->RemoteIP);
		Console::WriteLine("[ Server ] Info : RemotePort => " + args->RemotePort);

		clientPort = args->RemotePort;
	}
	catch (Exception ^ex)
	{
		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
	}
}

void TcpServer_ReceivedData(Object ^src, ReceivedArgument ^args)
{
	try
	{
		Console::WriteLine("[ MainForm ] System : Received data => " + args->SocketMessage->GetMessage());
	}
	catch (Exception ^ex)
	{
		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
	}
}

void TcpServer_DisconnectChange(Object ^src, ConnectChangedArgument ^args)
{
	try
	{
		Console::WriteLine("[ Server ] System : A client had disconnected to Server.");
		Console::WriteLine("[ Server ] Info : LocalIP => " + args->LocalIP);
		Console::WriteLine("[ Server ] Info : LocalPort => " + args->LocalPort);
		Console::WriteLine("[ Server ] Info : RemoteIP => " + args->RemoteIP);
		Console::WriteLine("[ Server ] Info : RemotePort => " + args->RemotePort);

		clientPort = 0;
	}
	catch (Exception ^ex)
	{
		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
	}
}



//void TcpClienter_ConnectChanged(Object ^src, ConnectChangedArgument ^args)
//{
//	try
//	{
//		Console::WriteLine("[ Server ] System : A client had connected to Server.");
//		Console::WriteLine("[ Server ] Info : LocalIP => " + args->LocalIP);
//		Console::WriteLine("[ Server ] Info : LocalPort => " + args->LocalPort);
//		Console::WriteLine("[ Server ] Info : RemoteIP => " + args->RemoteIP);
//		Console::WriteLine("[ Server ] Info : RemotePort => " + args->RemotePort);
//
//		clientPort = args->RemotePort;
//	}
//	catch (Exception ^ex)
//	{
//		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
//	}
//}
//
//void TcpClienter_ReceivedData(Object ^src, ReceivedArgument ^args)
//{
//	try
//	{
//		Console::WriteLine("[ MainForm ] System : Received data => " + args->SocketMessage->GetMessage());
//	}
//	catch (Exception ^ex)
//	{
//		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
//	}
//}
//
//void TcpClientor_DisconnectChange(Object ^src, ConnectChangedArgument ^args)
//{
//	try
//	{
//		Console::WriteLine("[ Server ] System : A client had disconnected to Server.");
//		Console::WriteLine("[ Server ] Info : LocalIP => " + args->LocalIP);
//		Console::WriteLine("[ Server ] Info : LocalPort => " + args->LocalPort);
//		Console::WriteLine("[ Server ] Info : RemoteIP => " + args->RemoteIP);
//		Console::WriteLine("[ Server ] Info : RemotePort => " + args->RemotePort);
//
//		clientPort = 0;
//	}
//	catch (Exception ^ex)
//	{
//		Console::WriteLine("[ MainForm ] Exception : " + ex->ToString());
//	}
//}



int main(array<System::String ^> ^args)
{
	// TcpServer object for test
	TcpServer ^tcpServer = gcnew TcpServer();
	tcpServer->Name = "TcpServer";
	tcpServer->LocalIP = "127.0.0.1";
	tcpServer->LocalPort = 5000;

	tcpServer->AcceptEvent += gcnew TcpServer::AcceptEventHandler(&TcpServer_ConnectAccept);
	tcpServer->ConnectChangedEvent += gcnew TcpServer::ConnectChangedEventHandler(&TcpServer_ConnectChanged);
	tcpServer->ReceivedEvent += gcnew TcpServer::ReceivedEventHandler(&TcpServer_ReceivedData);
	tcpServer->DisconnectedEvent += gcnew TcpServer::DisconnectChangedEventHandler(&TcpServer_DisconnectChange);
	
	tcpServer->Start();


	Thread::Sleep(10000);
	tcpServer->SendMessage("127.0.0.1", clientPort, "Hello Server!");

	// ==================== For chat message test ===================== //
	/*
	Thread::Sleep(20000);

	ISocketMessage ^recvMsg1 = tcpServer->ChatMessage("127.0.0.1", clientPort, "Hello Server!");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg1->GetMessage());

	Thread::Sleep(10000);

	ISocketMessage ^recvMsg2 = tcpServer->ChatMessage("127.0.0.1", clientPort, "The second message.");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg2->GetMessage());

	Thread::Sleep(10000);

	ISocketMessage ^recvMsg3 = tcpServer->ChatMessage("127.0.0.1", clientPort, "The last message.");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg3->GetMessage());*/


	// TcpClienter object for test
	/*TcpClienter ^tcpClientor = gcnew TcpClienter();
	tcpClientor->Name = "TcpServer";
	tcpClientor->LocalIP = "127.0.0.1";
	tcpClientor->LocalPort = 5001;
	tcpClientor->RemoteIP = "127.0.0.1";
	tcpClientor->RemotePort = 5000;

	tcpClientor->ConnectChangedEvent += gcnew TcpClienter::ConnectChangedEventHandler(&TcpClienter_ConnectChanged);
	tcpClientor->ReceivedEvent += gcnew TcpClienter::ReceivedEventHandler(&TcpClienter_ReceivedData);
	tcpClientor->DisconnectedEvent += gcnew TcpServer::DisconnectChangedEventHandler(&TcpClientor_DisconnectChange);

	tcpClientor->Start();



	// ==================== For chat message test ===================== //
	Thread::Sleep(10000);
	
	ISocketMessage ^recvMsg1 = tcpClientor->ChatMessage("127.0.0.1", 5000, "Hello Server!");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg1->GetMessage());
	
	Thread::Sleep(10000);

	ISocketMessage ^recvMsg2 = tcpClientor->ChatMessage("127.0.0.1", 5000, "The second message.");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg2->GetMessage());
	
	Thread::Sleep(10000);

	ISocketMessage ^recvMsg3 = tcpClientor->ChatMessage("127.0.0.1", 5000, "The last message.");
	Console::WriteLine("[ MainForm ] System : Received data => " + recvMsg3->GetMessage());*/


	Console::ReadKey(true);
    return 0;
}


