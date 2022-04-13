#include "GameWorld.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;
using namespace std::chrono;

GameWorld::GameWorld()
{
	this->m_inGame = false;
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

	nlohmann::json player;

	// loop through all game objects
	for (auto& gameObject : gameObjects)
	{
		if (gameObject.count("isPlayer") <= 0 || !gameObject.at("isPlayer")) {
			spawnDefinedGameObject(gameObject);
		}
		else {
			this->m_playerDef = gameObject;
		}
	}

	// init camera controller
	nlohmann::json cameraControllerObj = world.at("cameraController");

	// spawn host player
	auto hostPlayer = spawnDefinedGameObject(this->m_playerDef);

	std::shared_ptr<Connection> hostConnection = std::make_shared<Connection>();
	hostConnection->id = "Server";
	hostConnection->ip = sf::IpAddress::getLocalAddress();
	this->m_players.emplace(hostConnection, hostPlayer.get());
	this->mp_currentPlayer = hostPlayer.get();

	int spawnIdx = 0;
	for (auto& playerPair : this->m_players)
	{
		if (spawnIdx < this->mp_playerSpawns.size()) {
			playerPair.second->setPosition(this->mp_playerSpawns[spawnIdx].position);
			spawnIdx++;
		}
	}

	this->mp_view = std::make_shared<sf::View>(view);
	CameraController cc(cameraControllerObj.at("cameraLerpSpeed"), this->mp_view);
	this->m_cameraControl = cc;

	// init game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->init();
	}

	this->m_cameraControl.setTarget(hostPlayer);
}

void GameWorld::initClient(std::string filePath, sf::View& view)
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

	// init camera controller
	nlohmann::json cameraControllerObj = world.at("cameraController");

	this->mp_view = std::make_shared<sf::View>(view);
	CameraController cc(cameraControllerObj.at("cameraLerpSpeed"), this->mp_view);
	this->m_cameraControl = cc;
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
	PlatformDataEngineWrapper::getNetworkHandler()->recieve(this);

	// network update
	// limit send rate
	if (PlatformDataEngineWrapper::getIsClient()) {
		if (this->m_packetClock.getElapsedTime().asMilliseconds() > 40) {
			PlatformDataEngineWrapper::getNetworkHandler()->process(this);

			m_packetClock.restart();
		}
	}

	// update tile objects
	this->mp_tileMap->update(dt, elapsedTime);

	// update game objects
	for (auto gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->update(dt, elapsedTime);
	}

	if (PlatformDataEngineWrapper::getIsClient()) {
		for (std::string name : this->mp_netToDestroy)
		{
			if (this->getGameObject(name) != nullptr)
			{
				this->getGameObject(name)->destroySelf();
			}
			this->mp_netToDestroy.resize(this->mp_netToDestroy.size() - 1);
		}
	}
	
	// remove deleted gameObjects
	for (auto it = this->mp_gameObjects.cbegin(); it != this->mp_gameObjects.cend();)
	{
		if (it->second->getDestroyed())
		{
			this->addNetToDestroy(it->second->getName());

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
	gameObject->setType(name);
	this->m_gameObjectDefinitions.emplace(name, gameObject);
}

std::shared_ptr<GameObject> GameWorld::spawnGameObject(std::string type, sf::Vector2f position, std::string name, bool noReplication)
{
	//sf::Clock timer;
	if (this->m_gameObjectDefinitions.count(type) > 0) {
		std::shared_ptr<GameObject> p_gameObject = std::make_shared<GameObject>(
			*this->getGameObjectDefs().at(type)
			);

		// since we're spawning something, it's not a definition
		p_gameObject->setIsDefinition(false);

		p_gameObject->setPosition(position);
		p_gameObject->registerComponentHierarchy(p_gameObject);

		if (name == "") {
			name = Utility::generate_uuid_v4();
		}

		p_gameObject->setName(name);
		p_gameObject->setIsUI(false);

		this->registerGameObject(
			name, p_gameObject
		);

		p_gameObject->init();

		//spdlog::info("Spawning object {} took: {}uS at position {}, {}", p_gameObject->getName(), timer.getElapsedTime().asMicroseconds(), position.x, position.y);
		if (!PlatformDataEngineWrapper::getIsClient() && !noReplication)
		{
			dynamic_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler())->replicateGameObject(p_gameObject.get());
		}

		if (PlatformDataEngineWrapper::getIsClient()) {
			if (name == dynamic_cast<Client*>(PlatformDataEngineWrapper::getNetworkHandler())->getConnection()->id) {
				this->mp_currentPlayer = p_gameObject.get();
				this->m_cameraControl.setTarget(p_gameObject);
			}
		}

		return p_gameObject;
	}
	else {
		return nullptr;
	}
}

std::string GameWorld::spawnPlayer(std::shared_ptr<Connection> conn)
{
	std::shared_ptr<GameObject> player = this->spawnDefinedGameObject(this->m_playerDef, conn->id);
	this->m_players.emplace(conn, player.get());
	player->setName(conn->id);

	player->setConnection(conn);
	player->init();

	for (std::shared_ptr<GameObject> child : player->getChildren())
	{
		child->init();
	}

	return player->getName();
}

std::shared_ptr<GameObject> GameWorld::spawnDefinedGameObject(nlohmann::json gameObject, std::string name)
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

	if (name == "") {
		name = Utility::generate_uuid_v4();
	}
	p_gameObject->setName(name);
	this->registerGameObject(name, p_gameObject);

	// spawn children
	for (nlohmann::json child : gameObject.at("children"))
	{
		std::shared_ptr<GameObject> childObj = spawnDefinedGameObject(child);
		childObj->setParent(p_gameObject);
		p_gameObject->addChild(childObj);
	}

	if (!PlatformDataEngineWrapper::getIsClient())
	{
		dynamic_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler())->replicateGameObject(p_gameObject.get());
	}

	return p_gameObject;
}
