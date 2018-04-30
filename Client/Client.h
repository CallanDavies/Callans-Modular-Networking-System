#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

enum GameMessages
{
	ID_SERVER_TEXT_MESSAGE = ID_USER_PACKET_ENUM + 1,
	ID_SERVER_SET_CLIENT_ID,
	ID_CLIENT_CLIENT_DATA
};

class Client : public aie::Application 
{
public:

	struct GameObject
	{
		glm::vec3 position;
		glm::vec4 colour;
	};


	Client();
	virtual ~Client();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	// Initilize the connection
	void handleNetworkConnection();
	void initaliseClientConnection();

	// Handle incoming packets
	void handleNetworkMessages();

	void onSetClientIDPacket(RakNet::Packet* packet);

	void sendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);

	void sendClientGameObject();

	void onReceivedClientDataPacket(RakNet::Packet* packet);
	
protected:

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;
	int nextClientID = 1;
	int m_MyClientID;

	GameObject m_myGameObject;

	RakNet::RakPeerInterface* m_pPeerInterface;

	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};