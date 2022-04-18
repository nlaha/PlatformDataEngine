#include "GameWorld.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;
using namespace std::chrono;
namespace fs = std::filesystem;

GameWorld::GameWorld()
{
	this->m_inGame = false;
	this->mp_currentPlayer = nullptr;
	this->m_spawnIdx = 0;

	m_youDiedText.setText("YOU DIED");
	m_youDiedText.setColor(sf::Color::Red);
	m_youDiedText.setScale({ 0.8f, 0.8f });
	m_respawnTimerText.setScale({ 0.2f, 0.2f });
}

/// <summary>
/// Initializes the game world with a world.json definition file
/// and a view as defined by the game wrapper
/// </summary>
/// <param name="filePath">the path to the world definition file</param>
/// <param name="view">a view that has been linked to a window</param>
void GameWorld::init(const std::string& filePath, sf::View& view, ApplicationMode appMode)
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

	if (world.count("tiledMapFile") > 0) {
		this->mp_tileMap = std::make_shared<TileMap>(world.at("tiledMapFile"));
	}

	// construct game object instances
	nlohmann::json gameObjects = world.at("gameObjects");

	nlohmann::json player;

	this->mp_view = std::make_shared<sf::View>(view);

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

	if (appMode != ApplicationMode::DEDICATED) {
		// init camera controller
		if (world.count("cameraController") > 0) {
			nlohmann::json cameraControllerObj = world.at("cameraController");

			// spawn host player
			std::shared_ptr<Connection> hostConnection = std::make_shared<Connection>();
			hostConnection->id = "Server";
			hostConnection->name = PlatformDataEngineWrapper::getPlayerName();
			hostConnection->ip = sf::IpAddress::getLocalAddress();
			hostConnection->port = 5660;
			hostConnection->state = PlayerState::ALIVE;

			spawnPlayer(hostConnection);

			if (PlatformDataEngineWrapper::getNetworkHandler() != nullptr)
			{
				PlatformDataEngineWrapper::getNetworkHandler()->setConnection(hostConnection);
			}

			CameraController cc(cameraControllerObj.at("cameraLerpSpeed"), this->mp_view);
			this->m_cameraControl = cc;

			this->m_cameraControl.setTarget(this->mp_currentPlayer);
		}
	}

	// init game objects
	for (auto& gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->init();
	}
}

void GameWorld::initClient(const std::string& filePath, sf::View& view)
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

	m_physFilter = std::make_shared<ContactFilter>();
	this->mp_physicsWorld->SetContactFilter(m_physFilter.get());
}

/// <summary>
/// Update loop, calls update on the tileMap, gameObjects and the camera controller
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time (since game started)</param>
void GameWorld::update(const float& dt, const float& elapsedTime)
{
	// network recieve, unlimited
	if (PlatformDataEngineWrapper::getNetworkHandler() != nullptr) {
		PlatformDataEngineWrapper::getNetworkHandler()->recieve(this);

		// network update
		// limit send rate
		if (this->m_packetClock.getElapsedTime().asMilliseconds() > 24) {
			if (this->mp_currentPlayer == nullptr && PlatformDataEngineWrapper::getNetworkHandler()->getConnection() != nullptr) {
				this->mp_currentPlayer = this->getGameObject(PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->id).get();
			}

			PlatformDataEngineWrapper::getNetworkHandler()->process(this);
			m_packetClock.restart();
		}
	}

	// update tile objects
	if (this->mp_tileMap != nullptr) {
		this->mp_tileMap->update(dt, elapsedTime);
	}

	// update game objects
	for (auto gameObjectPair : this->mp_gameObjects)
	{
		gameObjectPair.second->update(dt, elapsedTime);

		if (this != PlatformDataEngineWrapper::getWorld().get()) {
			return;
		}
	}

	garbageCollect();

	// update camera
	this->m_cameraControl.update(dt, elapsedTime);

	if (PlatformDataEngineWrapper::getNetworkHandler() != nullptr) {
		if (PlatformDataEngineWrapper::getNetworkHandler()->getConnection() != nullptr) {
			if (PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->state == PlayerState::DEAD)
			{
				m_youDiedText.setPosition(PlatformDataEngineWrapper::getWindowCenter());
				m_respawnTimerText.setText(fmt::format("Respawning in: {:.2f}", 
					10.0f - PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->respawnTimer.getElapsedTime().asSeconds()));
				m_respawnTimerText.setPosition({ 
					PlatformDataEngineWrapper::getWindowCenter().x,
					PlatformDataEngineWrapper::getWindowCenter().y + 20
				});
			}
		}
	}
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
	if (this->mp_tileMap != nullptr) {
		target.draw(*this->mp_tileMap, states);
	}

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

	if (PlatformDataEngineWrapper::getNetworkHandler() != nullptr) {
		if (PlatformDataEngineWrapper::getNetworkHandler()->getConnection() != nullptr) {
			if (PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->state == PlayerState::DEAD)
			{
				target.draw(this->m_youDiedText, states);
				target.draw(this->m_respawnTimerText, states);
			}
		}
	}
}

/// <summary>
/// Registers a game object with a name 
/// (places it in the world's gameObjects map)
/// </summary>
/// <param name="name">a unique name for the gameObject</param>
/// <param name="gameObject">a pointer to the gameObject</param>
void GameWorld::registerGameObject(const std::string& name, std::shared_ptr<GameObject> gameObject)
{
	this->mp_gameObjects.emplace(name, gameObject);

	if (this->mp_gameObjects.count(name) <= 0)
	{
		spdlog::error("Failed to register game object {}", name);
	}
}

/// <summary>
/// Registers a gameObject definition, similar to registerGameObject() 
/// but it stores an actual gameObject rather than a pointer
/// </summary>
/// <param name="name">a unique name for the gameObject definition</param>
/// <param name="gameObject">the gameObject "template" definition</param>
void GameWorld::registerGameObjectDefinition(const std::string& name, std::shared_ptr<GameObject> gameObject)
{
	gameObject->setType(name);
	this->m_gameObjectDefinitions.emplace(name, gameObject);
}

void GameWorld::loadGameObjectDefinitions()
{
	// load game objects definitions from gameObjects/*.json
	// loop through all json files in game/gameObjects
	// and create game objects from them
	const fs::path gameObjectPath("./game/gameObjects/");

	for (const auto& entry : fs::directory_iterator(gameObjectPath)) {
		const auto filenameStr = entry.path().filename().string();
		if (entry.is_regular_file()) {
			if (entry.path().extension() == ".json")
			{
				// we've found a gameObject definition
				spdlog::info("Loading GameObject definition: {}", entry.path().string());
				std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>(true);
				gameObject->loadDefinition(entry.path().string());
				this->registerGameObjectDefinition(entry.path().filename().replace_extension("").string(), gameObject);
			}
		}
	}
}

std::shared_ptr<GameObject> GameWorld::spawnGameObject(const std::string& type, sf::Vector2f position, std::string name, bool noReplication)
{
	sf::Clock timer;
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
			spdlog::debug("Creating new UUID for object: {}", name);
		}

		p_gameObject->setId(name);
		p_gameObject->setIsUI(false);

		this->registerGameObject(
			name, p_gameObject
		);

		if (PlatformDataEngineWrapper::getIsClient()) {
			if (name == dynamic_cast<Client*>(PlatformDataEngineWrapper::getNetworkHandler())->getConnection()->id) {
				this->mp_currentPlayer = p_gameObject.get();
				this->m_cameraControl.setTarget(p_gameObject.get());
				spdlog::info("Setting current player to {}", p_gameObject->getId());
			}
		}

		p_gameObject->init();

		//spdlog::info("Spawning object {} took: {}uS at position {}, {}", p_gameObject->getId(), timer.getElapsedTime().asMicroseconds(), position.x, position.y);

		if (noReplication) {
			p_gameObject->setNetworked(false);
		}
		else {
			p_gameObject->setNetworked(true);
		}

		return p_gameObject;
	}
	else {
		return nullptr;
	}
}

std::string GameWorld::spawnPlayer(std::shared_ptr<Connection> conn)
{
	spdlog::info("Spawning player for connection: {}:{} - {}", conn->ip.toString(), conn->port, conn->id);

	std::shared_ptr<GameObject> player = this->spawnDefinedGameObject(this->m_playerDef, conn->id);
	this->m_players.emplace(conn, player.get());

	player->setPosition(this->mp_playerSpawns[this->m_spawnIdx].position);
	if (this->m_spawnIdx < this->mp_playerSpawns.size()) {
		this->m_spawnIdx++;
	}
	else {
		this->m_spawnIdx = 0;
	}

	if (!PlatformDataEngineWrapper::getIsClient()) {
		if (conn->id == "Server")
		{
			this->mp_currentPlayer = player.get();
		}
	}

	player->setName(conn->name);
	player->setId(conn->id);
	player->setNetworked(true);

	player->setConnection(conn);
	player->init();

	for (std::shared_ptr<GameObject> child : player->getChildren())
	{
		child->init();
	}

	return player->getId();
}

std::shared_ptr<GameObject> GameWorld::spawnDefinedGameObject(nlohmann::json gameObject, std::string name)
{
	std::shared_ptr<GameObject> p_gameObject = std::make_shared<GameObject>(
		*this->m_gameObjectDefinitions.at(gameObject.at("type"))
		);

	sf::Vector2f pos;
	if (gameObject.at("transform").at("x") != "centered") {
		pos.x = gameObject.at("transform").at("x");
	}
	else {
		pos.x = this->mp_view->getSize().x / 2.0f;
	}

	if (gameObject.at("transform").at("y") != "centered") {
		pos.y = gameObject.at("transform").at("y");
	}
	else {
		pos.y = this->mp_view->getSize().y / 2.0f;
	}

	p_gameObject->setPosition(
		pos.x,
		pos.y
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
	p_gameObject->setId(name);
	p_gameObject->setNetworked(true);

	this->registerGameObject(name, p_gameObject);

	// spawn children
	for (nlohmann::json child : gameObject.at("children"))
	{
		std::shared_ptr<GameObject> childObj = spawnDefinedGameObject(child);
		childObj->setParent(p_gameObject);
		p_gameObject->addChild(childObj);
	}


	return p_gameObject;
}

void GameWorld::garbageCollect()
{
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
	if (body != nullptr) {
		while (body->GetUserData().pointer != 0 && body->GetNext() != nullptr)
		{
			b2Body* next = body->GetNext();
			if (reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer)->destroyed == true) {
				delete reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer);
				this->getPhysWorld()->DestroyBody(body);
			}

			body = next;
		}
	}
}
