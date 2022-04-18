#include "Server.h"
#include "Utility.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

Server::Server()
{
	spdlog::info("Running in SERVER mode!");

	std::stringstream portSS(PlatformDataEngineWrapper::HostConfig::port);
	std::stringstream ipSS(PlatformDataEngineWrapper::HostConfig::ip);

	portSS >> this->m_port;
	ipSS >> this->m_ip;

	this->m_socket.setBlocking(false);
}

void Server::start()
{
	// bind the m_socket to a port
	if (m_socket.bind(this->m_port, this->m_ip) != sf::Socket::Done)
	{
		spdlog::error("Could not bind socket to port {}", this->m_port);
	}
	else {
		spdlog::info("Server started, listening on port {}", this->m_port);
	}
}

void Server::stop()
{
}

void Server::process(GameWorld* world)
{
	if (this->m_clientConnection->state == PlayerState::DEAD)
	{
		if (this->m_clientConnection->respawnTimer.getElapsedTime().asSeconds() > 10)
		{
			spdlog::info("Spawning player {}", this->m_clientConnection->name);
			this->m_clientConnection->state = PlayerState::ALIVE;
			std::string playerId = world->spawnPlayer(this->m_clientConnection);
			world->getCameraController().setTarget(world->getGameObject(playerId).get());
		}
	}

	PDEPacket packet;
	bool isNetworked = false;
	for (std::shared_ptr<Connection> conn : this->m_connections)
	{
		if (conn->state == PlayerState::DEAD)
		{
			if (conn->respawnTimer.getElapsedTime().asSeconds() > 10)
			{
				spdlog::info("Spawning player {}", conn->name);
				conn->state = PlayerState::ALIVE;
				std::string playerId = world->spawnPlayer(conn);
			}
		}

		// send update data
		packet = PDEPacket(PDEPacket::SendUpdates);
		conn->networkSerialize(packet);
		packet << static_cast<sf::Uint32>(world->getGameObjects().size());
		for (const auto& gameObjectPair : world->getGameObjects())
		{
			isNetworked = gameObjectPair.second->getNetworked();
			packet << isNetworked;
			if (isNetworked) {
				if (gameObjectPair.second->getId() == "")
				{
					spdlog::error("Update packet is malformed!");
				}
				packet
					<< gameObjectPair.second->getType()
					<< gameObjectPair.second->getPosition().x
					<< gameObjectPair.second->getPosition().y
					<< gameObjectPair.second->getId();
				if (gameObjectPair.second->getHasBeenSent(conn->id)) {
					spdlog::debug("Sending existing object {}", gameObjectPair.second->getId());
					gameObjectPair.second->networkSerialize(packet);
				}
				else {
					spdlog::debug("Sending new object {}", gameObjectPair.second->getId());
					gameObjectPair.second->networkSerializeInit(packet);
					gameObjectPair.second->setHasBeenSent(conn->id);
				}
			}
			else {
				spdlog::debug("Object is not networked! {}", gameObjectPair.second->getId());
			}
		}

		m_socket.send(packet, conn->ip, conn->port);
	}
}

void Server::recieve(GameWorld* world)
{
	PDEPacket packet;
	sf::IpAddress clientIp;
	unsigned short clientPort;

	m_socket.receive(packet, clientIp, clientPort);

	// player connected
	if (packet.flag() == PDEPacket::Connect) {
		// spawn new player on server
		std::string name;
		packet >> name;
		std::shared_ptr<Connection> connection = std::make_shared<Connection>();
		connection->ip = clientIp;
		connection->id = Utility::generate_uuid_v4();
		connection->name = name;
		connection->port = clientPort;
		connection->state = PlayerState::ALIVE;

		spdlog::info("A player has connected: {}:{} - {}", clientIp.toString(), clientPort, connection->id);
		std::string playerId = world->spawnPlayer(connection);

		// send connected message back
		PDEPacket connectedPacket(PDEPacket::Connected);
		connectedPacket << connection->id;
		m_socket.send(connectedPacket, clientIp, clientPort);

		this->m_connections.push_back(connection);
	}
	else {
		unsigned short numAxis = 0;
		unsigned short numButtons = 0;
		sf::Int8 idx = 0;
		sf::Int8 value = 0;
		bool valueBool = false;
		std::string clientId;
		packet >> clientId;
		bool isNetworked;

		std::shared_ptr<Connection> connection = this->findConnection(clientIp, clientId);
		if (connection != nullptr) {
			GameObject* player = world->getPlayer(connection);

			std::shared_ptr<InputList> clientInputs = nullptr;

			switch (packet.flag())
			{
			case PDEPacket::UserInput:
				// recieve input data
				clientInputs = this->m_inputManagers.at(connection);

				packet >> numAxis >> numButtons;

				for (unsigned short i = 0; i < numAxis; i++)
				{
					packet >> idx >> value;
					for (const auto& input : clientInputs->inputs) {
						dynamic_cast<NetworkInputManager*>(input)->setAxis(idx, value);
					}

				}

				for (unsigned short i = 0; i < numButtons; i++)
				{
					packet >> idx >> valueBool;
					for (const auto& input : clientInputs->inputs) {
						dynamic_cast<NetworkInputManager*>(input)->setButton(idx, valueBool);
					}

				}

				float mouseX;
				float mouseY;
				packet >> mouseX >> mouseY;
				for (const auto& input : clientInputs->inputs) {
					dynamic_cast<NetworkInputManager*>(input)->setMouse(sf::Vector2f(mouseX, mouseY));
				}

				break;

			case PDEPacket::Disconnect:
				this->m_connections.erase(std::remove(this->m_connections.begin(),
					this->m_connections.end(), findConnection(clientIp, clientId)));

				if (world->getGameObject(clientId) != nullptr) {
					world->getGameObject(clientId)->destroySelf();
					world->getPlayers().erase(findConnection(clientIp, clientId));
				}

				packet = PDEPacket(PDEPacket::Disconnected);
				packet << connection->id;
				m_socket.send(packet, clientIp, clientPort);

				spdlog::info("Player {} disconnected", clientId);

				break;
			}
		}
	}
}

void Server::broadcastObjectHealth(const std::string& objName, float health)
{
	// override timer for death packets
	if (this->m_broadcastCooldown.getElapsedTime().asMilliseconds() > 100 || health <= 0) {
		for (std::shared_ptr<Connection> conn : this->m_connections)
		{
			if (objName == conn->id) {
				// we're broadcasting a player's health
				if (health <= 0) {
					// player has died
					conn->state = PlayerState::DEAD;
					conn->respawnTimer.restart();
					spdlog::info("Player {} has died!", conn->name);
				}
			}

			PDEPacket packet(PDEPacket::SetObjectHealth);
			packet << objName << health;
			m_socket.send(packet, conn->ip, conn->port);
		}
		this->m_broadcastCooldown.restart();
	}
}

std::shared_ptr<Connection> Server::findConnection(sf::IpAddress ip, std::string id)
{
	for (std::shared_ptr<Connection> conn : this->m_connections)
	{
		if (conn->id == id && conn->ip == ip) {
			return conn;
		}
	}
	return nullptr;
}

