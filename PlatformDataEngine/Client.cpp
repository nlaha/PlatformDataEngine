#include "Client.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

Client::Client()
{
	spdlog::info("Running in CLIENT mode!");

	this->m_serverPort = 5660;
	this->m_serverIp = sf::IpAddress("localhost");
	this->m_socket.setBlocking(false);
	this->m_isConnecting = false;
	this->m_isConnected = false;
}

void Client::start()
{	
}

void Client::stop()
{
}

void Client::process(GameWorld* world)
{
	if (!this->m_isConnecting) {
		// try to connect to a server

		PDEPacket connectRequest(PDEPacket::Connect);
		const auto status = m_socket.send(connectRequest, this->m_serverIp, this->m_serverPort);
		if (status == sf::Socket::Error) {
			spdlog::error("Error connecting to server: {}:{}", this->m_serverIp.toString(), m_serverPort);
			exit(1);
		}
		else {
			this->m_isConnecting = true;
		}
	}
	else {
		
		if (this->m_isConnected) {
			// send out our current inputs
			PDEPacket inputPacket;
			inputPacket << this->m_clientConnection->id;
			PlatformDataEngineWrapper::getPlayerInputManager()->serializeInputs(inputPacket);
			if (m_socket.send(inputPacket, this->m_serverIp, m_serverPort) != sf::Socket::Done)
			{
				spdlog::warn("Failed to send input packet!");
			}

			PDEPacket updatesPacket(PDEPacket::RequestUpdates);
			updatesPacket << this->m_clientConnection->id;
			if (m_socket.send(updatesPacket, this->m_serverIp, m_serverPort) != sf::Socket::Done)
			{
				spdlog::warn("Failed to send update request packet!");
			}
		}


	}
}

void Client::recieve(GameWorld* world)
{
	PDEPacket packet;
	// we're connected, start processing game packets
	sf::IpAddress newServerIp;
	unsigned short newServerPort;
	const auto status = m_socket.receive(packet, newServerIp, newServerPort);
	if (status == sf::Socket::Status::Done) {

		std::string objType = "";
		sf::Vector2f objPos;
		std::string objName = "";
		std::shared_ptr<GameObject> obj = nullptr;
		std::string objConnId = "";
		std::string parentName = "";
		sf::Uint32 numObjs = 0;

		switch (packet.flag())
		{
		case PDEPacket::SpawnGameObject:
			packet >> objType;
			packet >> objPos.x >> objPos.y;
			packet >> objName;
			//spdlog::info("Spawning gameObject of type {}, at ({}, {})", objType, objPos.x, objPos.y);
			obj = world->spawnGameObject(objType, objPos, objName);
			break;

		case PDEPacket::SpawnChild:
			packet >> parentName;
			packet >> objType;
			packet >> objPos.x >> objPos.y;
			packet >> objName;
			//spdlog::info("Spawning child gameObject of type {} on {}", objType, parentName);
			obj = world->spawnGameObject(objType, objPos, objName);
			if (world->getGameObject(parentName) != nullptr) {
				std::shared_ptr<GameObject> parent = world->getGameObject(parentName);
				parent->addChild(obj);
				obj->setParent(parent);
			}
			break;

		case PDEPacket::UpdateGameObject:
			packet >> objName;
			//spdlog::info("Moving {} to position ({}, {})", objName, objPos.x, objPos.y);
			if (world->getGameObject(objName) != nullptr) {
				world->getGameObject(objName)->networkDeserialize(packet);
			}
			break;

		//case PDEPacket::GarbageCollect:
		//	packet >> numObjs;
		//	for (size_t i = 0; i < numObjs; i++)
		//	{
		//		packet >> objName;
		//		world->addNetToDestroy(objName);
		//	}
		//	break;

		case PDEPacket::Connected:

			this->m_clientConnection = std::make_shared<Connection>();
			packet >> this->m_clientConnection->id;
			this->m_clientConnection->ip = sf::IpAddress::getLocalAddress();
			this->m_isConnected = true;
			world->setInGame(true);
			spdlog::info("Client connected to server: {}:{}", m_serverIp.toString(), m_serverPort);

			break;

		default:
			break;
		}
	}
}
