#include "GameWorld.h"

using namespace PlatformDataEngine;

GameWorld::GameWorld()
{

}

void GameWorld::init(std::string filePath, sf::View& view)
{
	// init physics world
	b2Vec2 gravity(0.0f, 25.0f);
	this->mp_physicsWorld = std::make_shared<b2World>(gravity);

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

void GameWorld::physicsUpdate(const float& dt, const float& elapsedTime)
{
	int velocityIterations = 6;
	int positionIterations = 2;
	float timeStep = dt * 3.0f;

	// simulate physics
	this->mp_physicsWorld->Step(timeStep, velocityIterations, positionIterations);
}

void GameWorld::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// draw tile map
	target.draw(*this->mp_tileMap, states);

	// draw game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		target.draw(*gameObjectPair.second, states);
	}
}

void GameWorld::registerGameObject(std::string name, std::shared_ptr<GameObject> gameObject)
{
	this->mp_gameObjects.emplace(name, gameObject);
}


void GameWorld::registerGameObjectDefinition(std::string name, GameObject& gameObject)
{
	this->m_gameObjectDefinitions.emplace(name, gameObject);
}