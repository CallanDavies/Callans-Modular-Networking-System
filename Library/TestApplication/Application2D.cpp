#include "Application2D.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"

Application2D::Application2D() 
{
	m_Client = new Client();
}

Application2D::~Application2D() 
{

}

bool Application2D::startup() 
{
	//Call Client Startup()
	m_Client->startup();

	// initialise gizmo primitive counts
	aie::Gizmos::create(10000, 10000, 10000, 10000);

	return true;
}

void Application2D::shutdown() 
{
	
	delete m_font;
	delete m_texture;
	delete m_shipTexture;
	delete m_2dRenderer;
}

void Application2D::update(float deltaTime) 
{
	//Call Client Update() after any move
	m_Client->update();
	m_timer += deltaTime;
	aie::Gizmos::clear();

	// input example
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_LEFT))
	{
		m_Client->m_myGameObject.position.x -= 100.0f * deltaTime;
		m_Client->sendClientGameObject();
	}
	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
	{
		m_Client->m_myGameObject.position.x += 100.0f * deltaTime;
		m_Client->sendClientGameObject();
	}

	if (input->isKeyDown(aie::INPUT_KEY_UP))
	{
		m_Client->m_myGameObject.position.y += 100.0f * deltaTime;
		m_Client->sendClientGameObject();
	}
	if (input->isKeyDown(aie::INPUT_KEY_DOWN))
	{
		m_Client->m_myGameObject.position.y -= 100.0f * deltaTime;
		m_Client->sendClientGameObject();
	}

	// drawing of the player circle
	aie::Gizmos::add2DCircle(m_Client->m_myGameObject.position, 24.0f, 32, m_Client->m_myGameObject.colour);

	//drawing of the other circle
	for (auto& otherClient : m_Client->m_otherClientGameObjects)
	{
		aie::Gizmos::add2DCircle(otherClient.second.position, 24.0f, 32, otherClient.second.colour);
	}


	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();

}

void Application2D::draw() 
{

	// wipe the screen to the background colour
	clearScreen();

	static float aspectRatio = 16 / 9.f;
	aie::Gizmos::draw2D(glm::ortho<float>(-640, 640, -640 / aspectRatio, 640 / aspectRatio, -1.0f, 1.0f)); // Pixel Perfect for 1280 / 720
}