#include "Server.h"
#include "Utility.h"

using namespace PlatformDataEngine;

Server::Server()
{
	spdlog::info("Running in SERVER mode!");

	this->m_port = 5660;
	this->m_ip = sf::IpAddress("localhost");
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
		spdlog::info("A player has connected with ip: {}", clientIp.toString());

		// spawn new player on server
		std::shared_ptr<Connection> connection = std::make_shared<Connection>();
		connection->ip = clientIp;
		connection->id = Utility::generate_uuid_v4();
		connection->port = clientPort;
		std::string playerId = world->spawnPlayer(connection);

		// send connected message back
		PDEPacket connectedPacket(PDEPacket::Connected);
		connectedPacket << connection->id;
		m_socket.send(connectedPacket, clientIp, clientPort);

		this->m_connections.push_back(connection);

		// send over players first
		for (const auto& gameObjectPair : world->getPlayers())
		{
			replicateGameObject(gameObjectPair.second);
		}

		// send over currently spawned game objects
		const auto players = world->getPlayers();
		for (const auto& gameObjectPair : world->getGameObjects())
		{
			// don't send over players a second time!
			int count = std::count_if(players.begin(), players.end(), [&]
			(std::pair<std::shared_ptr<Connection>, GameObject*> go) {
				return go.second == gameObjectPair.second.get();
			});

			if (count == 0) {
				replicateGameObject(gameObjectPair.second.get());
			}
		}

	}
	else {
		unsigned short numAxis = 0;
		unsigned short numButtons = 0;
		sf::Int8 idx = 0;
		sf::Int8 value = 0.0f;
		bool valueBool = false;
		std::string clientId;
		packet >> clientId;
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

			case PDEPacket::RequestUpdates:
				// send move data
				for (const auto& gameObjectPair : world->getGameObjects())
				{
					if (gameObjectPair.second->getNetworked()) {
						PDEPacket packet(PDEPacket::UpdateGameObject);
						packet << gameObjectPair.second->getName();
						gameObjectPair.second->networkSerialize(packet);

						m_socket.send(packet, clientIp, clientPort);
					}
				}

				//PDEPacket destroyPacket(PDEPacket::GarbageCollect);
				//destroyPacket << static_cast<sf::Uint32>(world->getNetToDestroy().size());
				//for (const auto& gameObjectToDestroy : world->getNetToDestroy())
				//{
				//	destroyPacket << gameObjectToDestroy;
				//	m_socket.send(destroyPacket, clientIp, clientPort);
				//}
				//world->clearNetDestroy();
				break;
			}
		}
	}
}

void Server::replicateGameObject(GameObject* newObject)
{
	for (std::shared_ptr<Connection> conn : this->m_connections) {
		PDEPacket spawnPacket(PDEPacket::SpawnGameObject);
		spawnPacket
			<< newObject->getType()
			<< newObject->getPosition().x
			<< newObject->getPosition().y
			<< newObject->getName();
		m_socket.send(spawnPacket, conn->ip, conn->port);

		for (std::shared_ptr<GameObject> child : newObject->getChildren())
		{
			replicateChild(child, newObject->getName());
		}
	}
}

void Server::replicateChild(std::shared_ptr<GameObject> child, std::string parent)
{
	for (std::shared_ptr<Connection> conn : this->m_connections) {
		PDEPacket spawnPacket(PDEPacket::SpawnChild);
		spawnPacket
			<< parent
			<< child->getType()
			<< child->getPosition().x
			<< child->getPosition().y
			<< child->getName();
		m_socket.send(spawnPacket, conn->ip, conn->port);
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
}

