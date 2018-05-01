#include "Client.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

Client::Client() {

}

Client::~Client() 
{
}

bool Client::startup() 
{
	m_myGameObject.position = glm::vec3(0, 0, 0);
	m_myGameObject.colour = glm::vec4(1, 0, 0, 1);

	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	//// create simple camera transforms
	//m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	//m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
	//									  getWindowWidth() / (float)getWindowHeight(),
	//									  0.1f, 1000.f);
	
	
	handleNetworkConnection();

	return true;
}

void Client::shutdown() 
{

	Gizmos::destroy();
}

void Client::update(float deltaTime) 
{

	// query time since application started
	float time = getTime();
	
	// wipe the gizmos clean for this frame
	Gizmos::clear();

	// ImGui setup
	ImGui::Begin("Chat");
	ImGui::ColorEdit3("Circle colour", glm::value_ptr(m_myGameObject.colour));
	static char inputText[128];
	ImGui::InputText("##input", inputText, sizeof(inputText));
	ImGui::SameLine();
	if (ImGui::Button("Send##input", ImVec2(50,20))) 
	{

		sendChatMessage(inputText);
	}

	ImGui::End();
	
	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();;

	if (input->isKeyDown(aie::INPUT_KEY_LEFT))
	{
		m_myGameObject.position.x -= 100.0f * deltaTime;
		sendClientGameObject();
	}
	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
	{
		m_myGameObject.position.x += 100.0f * deltaTime;
		sendClientGameObject();
	}

	if (input->isKeyDown(aie::INPUT_KEY_UP))
	{
		m_myGameObject.position.y += 100.0f * deltaTime;
		sendClientGameObject();
	}
	if (input->isKeyDown(aie::INPUT_KEY_DOWN))
	{
		m_myGameObject.position.y -= 100.0f * deltaTime;
		sendClientGameObject();
	}

	// drawing of the player circle
	Gizmos::add2DCircle(m_myGameObject.position, 24.0f, 32, m_myGameObject.colour);

	//drawing of the other circle
	for (auto& otherClient : m_otherClientGameObjects)
	{
		Gizmos::add2DCircle(otherClient.second.position, 24.0f, 32, otherClient.second.colour);
	}


	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();

	// 
	handleNetworkMessages();
}

void Client::draw() 
{

	// wipe the screen to the background colour
	clearScreen();

	//// update perspective in case window resized
	//m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
	//									  getWindowWidth() / (float)getWindowHeight(),
	//									  0.1f, 1000.f);

	//Gizmos::draw(m_projectionMatrix * m_viewMatrix);

	static float aspectRatio = 16 / 9.f;
	aie::Gizmos::draw2D(glm::ortho<float>(-640, 640, -640 / aspectRatio, 640 / aspectRatio, -1.0f, 1.0f)); // Pixel Perfect for 1280 / 720

}

void Client::handleNetworkConnection()
{
	// Initialize the Raknet peer interface first
	m_pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	initaliseClientConnection();
}

void Client::initaliseClientConnection()
{
	// Create a socket descriptor to describe this connection
	// No data needed, as we will be connecting to a server
	RakNet::SocketDescriptor sd;

	// Now call startup - max of 1 connections (to the server)
	m_pPeerInterface->Startup(1, &sd, 1);

	std::cout << "Connecting to server at: " << IP << std::endl;

	// Now call connect to attempt to connect to the given server
	RakNet::ConnectionAttemptResult res = m_pPeerInterface->Connect(IP, PORT, nullptr, 0);

	// Finally, check to see if we connected and if not, throw an error
	if (res != RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		std::cout << "Unable to start connection, Error number: " << res << std::endl;
	}
}

void Client::handleNetworkMessages()
{
	RakNet::Packet* packet = nullptr;

	for (packet = m_pPeerInterface->Receive(); packet;
		m_pPeerInterface->DeallocatePacket(packet),
		packet = m_pPeerInterface->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			std::cout << "Another client has disconnected.\n";
			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Another client has lost the connection.\n";
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			std::cout << "Another client has connected.\n";
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Our connection request has been accepted.\n";
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			std::cout << "The server is full.\n";
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "We have been disconnected.\n";
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Connection lost.\n";
			break;
		case ID_SERVER_TEXT_MESSAGE:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			RakNet::RakString str;
			bsIn.Read(str);
			std::cout << str.C_String() << std::endl;
			break;
		}
		case ID_NEW_INCOMING_CONNECTION:
		{
			break;
		}
		case ID_SERVER_SET_CLIENT_ID:
			onSetClientIDPacket(packet);
			break;

		case ID_CLIENT_CLIENT_DATA:
		{
			onReceivedClientDataPacket(packet);

			RakNet::BitStream bs(packet->data, packet->length, false);
			m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

			break;
		}

		case ID_CLIENT_CHAT_MESSAGE: {

			// Skip over the enumerator identifier
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			// Read in message
			RakNet::RakString rakString;
			bsIn.Read(rakString);

			// Assign member variables with newely read in data
			m_mostRecentMessageReceived = rakString.C_String();


			std::cout << m_mostRecentMessageReceived.c_str() << std::endl;
			break;
		}
		


		default:
			std::cout << "Received a message with an unknown id: " << packet->data[0];
			break;
		}
	}
}


void Client::sendClientGameObject()
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_DATA);
	bs.Write(m_MyClientID);
	bs.Write((char*)&m_myGameObject, sizeof(GameObject));

	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::onReceivedClientDataPacket(RakNet::Packet * packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	int clientID;
	bsIn.Read(clientID);

	// If the clientID does not match our ID, we need to update our client GameObject information.
	if (clientID != m_MyClientID)
	{
		GameObject clientData;
		bsIn.Read((char*)&clientData, sizeof(GameObject));

		m_otherClientGameObjects[clientID] = clientData;
	}
}

void Client::sendChatMessage(std::string message)
{
	// cast message to rakstring
	RakNet::RakString rakString;
	rakString.Set(message.c_str());

	// create packet
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CHAT_MESSAGE);
	bs.Write(rakString);

	// send packet to everyone
	m_pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::onSetClientIDPacket(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(m_MyClientID);

	std::cout << "Set my client ID to: " << m_MyClientID << std::endl;
}

void Client::sendNewClientID(RakNet::RakPeerInterface * pPeerInterface, RakNet::SystemAddress & address)
{
}

