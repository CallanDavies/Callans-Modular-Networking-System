#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

int nextClientID = 1;

enum GameMessages
{
	ID_SERVER_TEXT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_SERVER_SET_CLIENT_ID,
	ID_CLIENT_CLIENT_DATA,
	ID_CLIENT_CHAT_MESSAGE
};

void sendClientPing(RakNet::RakPeerInterface* pPeerInterface)
{
	while (true)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
		bs.Write("Ping!");

		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
			RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);
	bs.Write(nextClientID);
	nextClientID++;

	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false);
}

void handleNetworkMessages(RakNet::RakPeerInterface* pPeerInterface)
{
	RakNet::Packet* packet = nullptr;

	for (packet = pPeerInterface->Receive(); packet;
		pPeerInterface->DeallocatePacket(packet),
		packet = pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_NEW_INCOMING_CONNECTION:
		{
			std::cout << "A connection has been made.\n";
			sendNewClientID(pPeerInterface, packet->systemAddress);
			break;
		}

		case ID_CLIENT_CHAT_MESSAGE: {
			
			RakNet::BitStream bs(packet->data, packet->length, false);
			pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
			break;
		}
		
		default:
			break;
		}
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main()
{
	
	
	const unsigned short PORT = 5456;
	RakNet::RakPeerInterface* pPeerInterface = nullptr;

	//startup the server, and start it listening to clients
	std::cout << "Starting up the server..." << std::endl;

	//initilize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();

	//create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, "127.0.0.1");

	//Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);

	//Startup a thread to ping clients every second
	//std::thread pingThread(sendClientPing, pPeerInterface);

	while (true) {

		handleNetworkMessages(pPeerInterface);
	}
	system("Pause");
}