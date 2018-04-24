#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>


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

protected:

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	GameObject m_myGameObject;

	RakNet::RakPeerInterface* m_pPeerInterface;

	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;
};