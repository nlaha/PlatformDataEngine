#include "GameWorld.h"

using namespace PlatformDataEngine;

GameWorld::GameWorld()
{

}

/// <summary>
/// Initializes the game world with a world.json definition file
/// and a view as defined by the game wrapper
/// </summary>
/// <param name="filePath">the path to the world definition file</param>
/// <param name="view">a view that has been linked to a window</param>
void GameWorld::init(std::string filePath, sf::View& view)
{
	// load world json file
	std::ifstream file(filePath);

	if (!file.is_open())
	{
		spdlog::error("Failed to open world file: {}", filePath);
		return;
	}
	spdlog::info("Loading world file: {}", filePath);

	nlohmann::json world;
	file >> world;

	this->mp_tileMap = std::make_shared<TileMap>(world.at("tiledMapFile"));

	// construct game object instances
	nlohmann::json gameObjects = world.at("gameObjects");

	// loop through all game objects
	for (auto& gameObject : gameObjects)
	{
		std::shared_ptr<GameObject> p_gameObject = std::make_shared<GameObject>(
			this->m_gameObjectDefinitions.at(gameObject.at("type"))
		);
		p_gameObject->setPosition(
			gameObject.at("transform").at("x"),
			gameObject.at("transform").at("y")
		);
		p_gameObject->setZlayer(
			gameObject.at("transform").at("z_layer")
		);
		p_gameObject->setRotation(
			gameObject.at("transform").at("rotation")
		);
		p_gameObject->setScale({
			gameObject.at("transform").at("scale"),
			gameObject.at("transform").at("scale")
			});
		p_gameObject->registerComponentHierarchy(p_gameObject);
		this->registerGameObject(gameObject.at("name"), p_gameObject);
	}

	// init camera controller
	nlohmann::json cameraControllerObj = world.at("cameraController");

	this->mp_view = std::make_shared<sf::View>(view);
	CameraController cc(cameraControllerObj.at("cameraLerpSpeed"), this->mp_view);
	this->m_cameraControl = cc;
	this->m_cameraControl.setTarget(this->getGameObject(cameraControllerObj.at("cameraLockObject")));

	// init game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->init();
	}
}

void PlatformDataEngine::GameWorld::initPhysics()
{
	// init physics world
	b2Vec2 gravity(0.0f, 25.0f);
	this->mp_physicsWorld = std::make_shared<b2World>(gravity);
}

/// <summary>
/// Update loop, calls update on the tileMap, gameObjects and the camera controller
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time (since game started)</param>
void GameWorld::update(const float& dt, const float& elapsedTime)
{
	// update tile objects
	this->mp_tileMap->update(dt, elapsedTime);

	// update game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->update(dt, elapsedTime);
	}

	// update camera
	this->m_cameraControl.update(dt, elapsedTime);
}

/// <summary>
/// Updates the physics simulation
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time (since game started)</param>
void GameWorld::physicsUpdate(const float& dt, const float& elapsedTime)
{
	int velocityIterations = 8;
	int positionIterations = 4;
	float timeStep = dt * 3.0f;

	// simulate physics
	this->mp_physicsWorld->Step(timeStep, velocityIterations, positionIterations);
}

/// <summary>
/// Draw call, never call explicitly
/// </summary>
/// <param name="target"></param>
/// <param name="states"></param>
void GameWorld::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// draw tile map
	target.draw(*this->mp_tileMap, states);

	// sort game objects by z layer
	// TODO: this could be done faster
	std::vector<std::shared_ptr<GameObject>> gameObjects;
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjects.push_back(gameObjectPair.second);
	}

	std::sort(gameObjects.begin(), gameObjects.end(), [](std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
		return a->getZlayer() < b->getZlayer();
	});

	// draw game objects
	for (auto& gameObjectPair : gameObjects)
	{
		target.draw(*gameObjectPair, states);
	}
}

/// <summary>
/// Registers a game object with a name 
/// (places it in the world's gameObjects map)
/// </summary>
/// <param name="name">a unique name for the gameObject</param>
/// <param name="gameObject">a pointer to the gameObject</param>
void GameWorld::registerGameObject(std::string name, std::shared_ptr<GameObject> gameObject)
{
	this->mp_gameObjects.emplace(name, gameObject);
}

/// <summary>
/// Registers a gameObject definition, similar to registerGameObject() 
/// but it stores an actual gameObject rather than a pointer
/// </summary>
/// <param name="name">a unique name for the gameObject definition</param>
/// <param name="gameObject">the gameObject "template" definition</param>
void GameWorld::registerGameObjectDefinition(std::string name, GameObject& gameObject)
{
	this->m_gameObjectDefinitions.emplace(name, gameObject);
}