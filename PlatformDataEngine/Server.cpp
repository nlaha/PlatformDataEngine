#include "Server.h"
#include "Utility.h"

using namespace PlatformDataEngine;

Server::Server()
{
	spdlog::info("Running in SERVER mode!");

	std::ifstream netFile("./game/network.json");

	nlohmann::json netConf;
	netFile >> netConf;

	this->m_port = netConf.at("port");
	this->m_ip = sf::IpAddress::getPublicAddress();
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
		std::shared_ptr<Connection> connection = std::make_shared<Connection>();
		connection->ip = clientIp;
		connection->id = Utility::generate_uuid_v4();
		connection->port = clientPort;

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
				world->clearNetDestroy();

				packet = PDEPacket(PDEPacket::Disconnected);
				packet << connection->id;
				m_socket.send(packet, clientIp, clientPort);

				spdlog::info("Player {} disconnected", clientId);

				break;

			case PDEPacket::RequestUpdates:

				// send update data
				packet = PDEPacket(PDEPacket::ResponseUpdates);
				packet << static_cast<sf::Uint32>(world->getGameObjects().size());
				for (const auto& gameObjectPair : world->getGameObjects())
				{
					isNetworked = gameObjectPair.second->getNetworked();
					packet << isNetworked;
					if (isNetworked) {
						if (gameObjectPair.second->getName() == "")
						{
							spdlog::error("Update packet is malformed!");
						}
						packet
							<< gameObjectPair.second->getType()
							<< gameObjectPair.second->getPosition().x
							<< gameObjectPair.second->getPosition().y
							<< gameObjectPair.second->getName();
						if (gameObjectPair.second->getHasBeenSent(clientId)) {
							spdlog::debug("Sending existing object {}", gameObjectPair.second->getName());
							gameObjectPair.second->networkSerialize(packet);
						}
						else {
							spdlog::debug("Sending new object {}", gameObjectPair.second->getName());
							gameObjectPair.second->networkSerializeInit(packet);
							gameObjectPair.second->setHasBeenSent(clientId);
						}
					}
					else {
						spdlog::debug("Object is not networked! {}", gameObjectPair.second->getName());
					}
				}

				packet << static_cast<sf::Uint32>(world->getNetToDestroy().size());
				for (std::string name : world->getNetToDestroy())
				{
					packet << name;
				}

				m_socket.send(packet, clientIp, clientPort);
				break;
			}
		}
	}
}

void Server::broadcastObjectHealth(std::string objName, float health)
{
	for (std::shared_ptr<Connection> conn : this->m_connections)
	{
		PDEPacket packet(PDEPacket::SetObjectHealth);
		packet << objName << health;
		m_socket.send(packet, conn->ip, conn->port);
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

