#include "GameWorld.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;
using namespace std::chrono;

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
			*this->m_gameObjectDefinitions.at(gameObject.at("type"))
		);
		p_gameObject->setPosition(
			gameObject.at("transform").at("x"),
			gameObject.at("transform").at("y")
		);
		p_gameObject->setOrigin(
			gameObject.at("transform").at("origin_x"),
			gameObject.at("transform").at("origin_y")
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

		if (gameObject.count("isUI") > 0) {
			p_gameObject->setIsUI(gameObject.at("isUI"));
		}
		else {
			p_gameObject->setIsUI(false);
		}
		p_gameObject->registerComponentHierarchy(p_gameObject);

		this->registerGameObject(gameObject.at("name"), p_gameObject);
	}

	for (auto& gameObject : gameObjects)
	{
		for (std::string child : gameObject.at("children"))
		{
			std::shared_ptr<GameObject> childObj = this->getGameObject(child);
			childObj->setParent(this->getGameObject(gameObject.at("name")));
			this->getGameObject(gameObject.at("name"))->addChild(childObj);
		}
	}

	// init camera controller
	nlohmann::json cameraControllerObj = world.at("cameraController");

	// later change this to some system that multiplayer supports
	this->mp_currentPlayer = this->getGameObject(world.at("playerObject")).get();

	if (this->mp_playerSpawns.size() > 0) {
		this->mp_currentPlayer->setPosition(this->mp_playerSpawns[0].position);
	}

	this->mp_view = std::make_shared<sf::View>(view);
	CameraController cc(cameraControllerObj.at("cameraLerpSpeed"), this->mp_view);
	this->m_cameraControl = cc;

	// init game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->init();
	}

	this->m_cameraControl.setTarget(this->getGameObject(cameraControllerObj.at("cameraLockObject")));
}

void GameWorld::initPhysics()
{
	// init physics world
	b2Vec2 gravity(0.0f, 9.0f);
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
	for (auto gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->update(dt, elapsedTime);
	}

	// remove deleted gameObjects
	for (auto it = this->mp_gameObjects.cbegin(); it != this->mp_gameObjects.cend();)
	{
		if (it->second->getDestroyed())
		{
			if (it->second.get() == this->mp_currentPlayer) {
				this->setPlayer(nullptr);
			}

			// no idea why I need to explicitly call the destructor here, clearly
			// something still has ownership idk
			if (it->second->findComponentOfType<PhysicsBody>().get() != nullptr) {
				it->second->findComponentOfType<PhysicsBody>()->~PhysicsBody();
			}

			this->mp_gameObjects.erase(it++);
		}
		else
		{
			++it;
		}
	}

	// destroy physics bodies that are queued for destruction
	b2Body* body = this->mp_physicsWorld->GetBodyList();
	while (body->GetUserData().pointer != 0 && body->GetNext() != nullptr)
	{
		b2Body* next = body->GetNext();
		if (reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer)->destroyed == true) {
			delete reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer);
			this->getPhysWorld()->DestroyBody(body);
		}

		body = next;
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
	int velocityIterations = 12;
	int positionIterations = 8;
	float timeStep = dt * 2.0f;

	// simulate physics
	this->mp_physicsWorld->Step(std::min(timeStep, 0.15f), velocityIterations, positionIterations);
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
	for (auto& gameObject : gameObjects)
	{
		if (!gameObject->getDestroyed()) {
			if (gameObject->getParent() == nullptr)
			{
				target.draw(*gameObject, states);
			}
		}
	}

	if (PlatformDataEngineWrapper::getIsDebugPhysics()) {
		this->mp_physicsWorld->DebugDraw();
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
void GameWorld::registerGameObjectDefinition(std::string name, std::shared_ptr<GameObject> gameObject)
{
	this->m_gameObjectDefinitions.emplace(name, gameObject);
}

std::shared_ptr<GameObject> GameWorld::spawnGameObject(std::string type, sf::Vector2f position)
{
	sf::Clock timer;

	std::shared_ptr<GameObject> p_gameObject = std::make_shared<GameObject>(
		*this->getGameObjectDefs().at(type)
	);

	// since we're spawning something, it's not a definition
	p_gameObject->setIsDefinition(false);

	p_gameObject->setPosition(position);
	p_gameObject->registerComponentHierarchy(p_gameObject);

	std::string name = Utility::generate_uuid_v4() + "%id%" + p_gameObject->getName();

	p_gameObject->setName(name);
	p_gameObject->setIsUI(false);

	this->registerGameObject(
		name, p_gameObject
	);

	p_gameObject->init();

	spdlog::info("Spawning object {} took: {}uS at position {}, {}", p_gameObject->getName(), timer.getElapsedTime().asMicroseconds(), position.x, position.y);

	return p_gameObject;
}