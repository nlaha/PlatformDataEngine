#include "Client.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

Client::Client()
{
	spdlog::info("Running in CLIENT mode!");

	std::ifstream netFile("./game/network.json");

	nlohmann::json netConf;
	netFile >> netConf;

	this->m_serverPort = netConf.at("port");
	this->m_serverIp = sf::IpAddress(std::string(netConf.at("ip")));
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
		bool isNetworked = false;
		float objHealth = 0.0f;
		bool hasRecievedBefore = false;
		std::vector<std::string> updatedObjects;

		switch (packet.flag())
		{

		case PDEPacket::ResponseUpdates:
			packet >> numObjs;
			for (size_t i = 0; i < numObjs; i++)
			{
				packet >> isNetworked;
				if (isNetworked) {
					packet >> objType;
					packet >> objPos.x >> objPos.y;
					packet >> objName;

					updatedObjects.push_back(objName);

					if (objName == "")
					{
						spdlog::error("Update packet is malformed!");
					}
					//spdlog::info("Moving {} to position ({}, {})", objName, objPos.x, objPos.y);
					if (world->getGameObject(objName) != nullptr) {
						world->getGameObject(objName)->networkDeserialize(packet);
					}
					else {
						obj = world->spawnGameObject(objType, objPos, objName);
						obj->networkDeserializeInit(packet);
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
			//	packet >> objName;
			//	if (world->getGameObject(objName) != nullptr) {
			//		if (world->getGameObject(objName)->getHealth() <= 0)
			//		{
			//			world->getGameObject(objName)->onDeath();
			//		}
			//		world->getGameObject(objName)->destroySelf();
			//	}
			//}

			break;

		case PDEPacket::SetObjectHealth:
			packet >> objName;
			packet >> objHealth;
			if (world->getGameObject(objName) != nullptr) {
				world->getGameObject(objName)->setHealth(objHealth);
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