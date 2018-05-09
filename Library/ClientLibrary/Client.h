#pragma once

#include <glm/mat4x4.hpp>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <imgui.h>
#include <string>
#include <map>

enum GameMessages
{
	ID_SERVER_TEXT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_SERVER_SET_CLIENT_ID,
	ID_CLIENT_CLIENT_DATA,
	ID_CLIENT_CHAT_MESSAGE
};

class Client
{
public:

	struct GameObject
	{
		glm::vec2 position;
		glm::vec4 colour;
	};

	virtual bool startup();

	virtual void update();

	// Initilize the connection
	void handleNetworkConnection();
	void initaliseClientConnection();

	// Handle incoming packets
	void handleNetworkMessages();

	void onSetClientIDPacket(RakNet::Packet* packet);

	void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);

	void sendClientGameObject();

	void sendChatMessage(std::string message);

	void onReceivedClientDataPacket(RakNet::Packet* packet);


	GameObject m_myGameObject;

	std::map<int, GameObject> m_otherClientGameObjects;
	
protected:

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;
	int nextClientID = 1;
	int m_MyClientID;


	RakNet::RakPeerInterface* m_pPeerInterface;

	std::string m_mostRecentMessageReceived;

	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};