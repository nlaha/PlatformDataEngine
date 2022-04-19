#include "Client.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

Client::Client()
{
	spdlog::info("Running in CLIENT mode!");

	std::stringstream portSS(PlatformDataEngineWrapper::JoinConfig::port);
	std::stringstream ipSS(PlatformDataEngineWrapper::JoinConfig::ip);

	portSS >> this->m_serverPort;
	ipSS >> this->m_serverIp;

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
		connectRequest << PlatformDataEngineWrapper::getPlayerName();
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
		std::string objId = "";
		std::shared_ptr<GameObject> obj = nullptr;
		std::string objConnId = "";
		std::string parentName = "";
		sf::Uint32 numObjs = 0;
		bool isNetworked = false;
		float objHealth = 0.0f;
		bool hasRecievedBefore = false;
		std::vector<std::string> updatedObjects;

		switch (packet.flag())
		{

		case PDEPacket::SendUpdates:
			this->m_clientConnection->networkDeserialize(packet);
			packet >> numObjs;
			for (size_t i = 0; i < numObjs; i++)
			{
				packet >> isNetworked;
				if (isNetworked) {
					packet >> objType;
					packet >> objPos.x >> objPos.y;
					packet >> objId;
					packet >> hasRecievedBefore;

					updatedObjects.push_back(objId);

					if (objId == "")
					{
						spdlog::error("Update packet is malformed!");
					}
					//spdlog::info("Moving {} to position ({}, {})", objId, objPos.x, objPos.y);
					if (hasRecievedBefore) {
						world->getGameObject(objId)->networkDeserialize(packet);
					}
					else {
						if (world->getGameObject(objId) == nullptr) {
							spdlog::debug("Creating object with ID: {}", objId);
							obj = world->spawnGameObject(objType, objPos, objId);
							obj->networkDeserializeInit(packet);
						}
						else {
							spdlog::warn("Skipped initialization for existing object: {} {}", objId, objType);
						}
					}
				}
			}

			//packet >> numObjs;

			for (const auto& gameObjectPair : world->getGameObjects())
			{
				if (gameObjectPair.second->getNetworked()) {
					if (std::find(updatedObjects.begin(),
						updatedObjects.end(),
						gameObjectPair.first) != updatedObjects.end())
					{
						// found object, don't destroy
					}
					else {
						// we didn't get an update, go ahead and destroy it
						gameObjectPair.second->onDeath();
					}
				}
			}

			//for (size_t i = 0; i < numObjs; i++)
			//{
			//	packet >> objId;
			//	if (world->getGameObject(objId) != nullptr) {
			//		if (world->getGameObject(objId)->getHealth() <= 0)
			//		{
			//			world->getGameObject(objId)->onDeath();
			//		}
			//		world->getGameObject(objId)->destroySelf();
			//	}
			//}

			break;

		case PDEPacket::SetObjectHealth:
			packet >> objId;
			packet >> objHealth;
			if (world->getGameObject(objId) != nullptr) {
				world->getGameObject(objId)->setHealth(objHealth);
			}

			if (objId == this->m_clientConnection->id)
			{
				if (this->m_clientConnection->health <= 0) {
					this->m_clientConnection->respawnTimer.restart();
				}
			}

			break;

		case PDEPacket::Connected:
			this->m_clientConnection = std::make_shared<Connection>();
			packet >> this->m_clientConnection->id;
			this->m_clientConnection->ip = sf::IpAddress::getPublicAddress();
			this->m_isConnected = true;
			world->setInGame(true);
			spdlog::info("Client connected to server: {}:{} - {}", m_serverIp.toString(), m_serverPort, this->m_clientConnection->id);

			break;

		case PDEPacket::Disconnected:
			this->m_clientConnection = nullptr;
			spdlog::info("Disconnected from server!");
			break;
		}
	}
}

void Client::disconnect()
{
	PDEPacket disconnectPk(PDEPacket::Disconnect);
	disconnectPk << this->m_clientConnection->id;
	if (m_socket.send(disconnectPk, this->m_serverIp, m_serverPort) != sf::Socket::Done)
	{
		spdlog::warn("Failed to send disconnect request packet!");
	}
}
