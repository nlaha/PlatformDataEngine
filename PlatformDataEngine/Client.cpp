#include "Client.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
Client::Client()
{
	spdlog::info("Running in CLIENT mode!");

	std::stringstream portSS(PlatformDataEngineWrapper::JoinConfig::port);
	std::stringstream ipSS(PlatformDataEngineWrapper::JoinConfig::ip);

	portSS >> this->m_serverPort;
	ipSS >> this->m_serverIp;
}

static Client* s_pCallbackInstance;
static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

void Client::PollConnectionStateChanges()
{
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}

/// <summary>
/// Starts the client,
/// doesn't do anything right now
/// </summary>
void Client::start()
{
	InitSteamDatagramConnectionSockets();

	m_pInterface = SteamNetworkingSockets();

	// Start connecting
	SteamNetworkingIPAddr serverAddr;
	serverAddr.Clear();
	serverAddr.ParseString(this->m_serverIp.toString().c_str());
	serverAddr.m_port = this->m_serverPort;

	spdlog::info("Connecting to server at {}:{}", this->m_serverIp.toString(), this->m_serverPort);
	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
	m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
	if (m_hConnection == k_HSteamNetConnection_Invalid)
		spdlog::error("Failed to create connection");
}

/// <summary>
/// Stops the client,
/// doesn't do anything right now
/// </summary>
void Client::stop()
{
}

void Client::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

	// What's the state of the connection?
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		// Print an appropriate message
		if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
		{
			// Note: we could distinguish between a timeout, a rejected connection,
			// or some other transport problem.
			spdlog::error("We sought the remote host, yet our efforts were met with defeat.  ({})", pInfo->m_info.m_szEndDebug);
		}
		else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
		{
			spdlog::warn("Alas, troubles beset us; we have lost contact with the host.  ({})", pInfo->m_info.m_szEndDebug);
		}
		else
		{
			// NOTE: We could check the reason code for a normal disconnection
			spdlog::info("The host hath bidden us farewell.  ({})", pInfo->m_info.m_szEndDebug);
		}

		// Clean up the connection.  This is important!
		// The connection is "closed" in the network sense, but
		// it has not been destroyed.  We must close it on our end, too
		// to finish up.  The reason information do not matter in this case,
		// and we cannot linger because it's already closed on the other end,
		// so we just pass 0's.
		m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		m_hConnection = k_HSteamNetConnection_Invalid;
		break;
	}

	case k_ESteamNetworkingConnectionState_Connecting:
		// We will get this callback when we start connecting.
		// We can ignore this.
		spdlog::info("Attempting to connect to server...");
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		spdlog::info("Connected to server OK");
		break;

	default:
		// Silences -Wswitch
		break;
	}
}

/// <summary>
/// Sends data to the server, should be called
/// in a fixed time period loop
/// </summary>
/// <param name="world">The game world</param>
void Client::process(GameWorld* world)
{
	if (this->m_clientConnection != nullptr) {
		// send out our current inputs
		PDEPacket inputPacket(PDEPacket::UserInput);
		inputPacket << this->m_clientConnection->id;
		PlatformDataEngineWrapper::getPlayerInputManager()->serializeInputs(inputPacket);

		SendPacketToServer(this->m_hConnection, inputPacket);
	}
}

/// <summary>
/// Checks for new data from the server and processes it
/// </summary>
/// <param name="world">The game world</param>
void Client::recieve(GameWorld* world)
{
	PollConnectionStateChanges();

	ISteamNetworkingMessage* pIncomingMsg = nullptr;
	int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
	if (numMsgs == 0) {
		return;
	}
	if (numMsgs < 0) {
		spdlog::error("Error checking for messages");
	}

	PDEPacket packet;
	packet.clear();
	packet.onReceive(pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);

	// We don't need this anymore.
	pIncomingMsg->Release();

	std::string objType = "";
	sf::Vector2f objPos;
	std::string objId = "";
	std::shared_ptr<GameObject> obj = nullptr;
	std::string objConnId = "";
	std::string parentName = "";
	sf::Uint32 numObjs = 0;
	bool isNetworked = true;
	float objHealth = 0.0f;
	bool hasRecievedBefore = false;
	std::vector<std::string> updatedObjects;

	//spdlog::warn("Flag: {}", packet.flag());

	//spdlog::info("Packet Size: {}", pIncomingMsg->m_cbSize);

	switch (packet.flag())
	{
	case PDEPacket::SendUpdates:
		this->m_clientConnection->networkDeserialize(packet);

		while(isNetworked)
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
					spdlog::error("Update packet is malformed! Size: {}, Data {}", packet.getDataSize(), fmt::ptr(packet.getData()));
				}
				//spdlog::info("Moving {} to position ({}, {})", objId, objPos.x, objPos.y);
				if (hasRecievedBefore && world->getGameObject(objId) != nullptr) {
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
			else {
				break;
			}
		}

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
					spdlog::info("Destryoing object {}", gameObjectPair.first);
					gameObjectPair.second->onDamage(0);
					gameObjectPair.second->onDeath();
				}
			}
		}

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
		world->setInGame(true);
		spdlog::info("Client connected to server: {}:{} - {}", m_serverIp.toString(), m_serverPort, this->m_clientConnection->id);

		break;

	case PDEPacket::Disconnected:
		this->m_clientConnection = nullptr;
		spdlog::info("Disconnected from server!");
		break;
	}
}

/// <summary>
/// Lets the server know we're disconnecting
/// </summary>
void Client::disconnect()
{
	m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
}
