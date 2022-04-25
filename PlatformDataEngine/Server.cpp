#include "Server.h"
#include "Utility.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
Server::Server(GameWorld* world)
{
	this->m_world = world;

	spdlog::info("Running in SERVER mode!");

	std::stringstream portSS(PlatformDataEngineWrapper::HostConfig::port);
	std::stringstream ipSS(PlatformDataEngineWrapper::HostConfig::ip);

	portSS >> this->m_port;
	ipSS >> this->m_ip;
}

static Server* s_pCallbackInstance;
static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

void Server::PollConnectionStateChanges()
{
	s_pCallbackInstance = this;
	m_pInterface->RunCallbacks();
}


/// <summary>
/// Starts the server
/// </summary>
void Server::start()
{
	InitSteamDatagramConnectionSockets();

	// Select instance to use.  For now we'll always use the default.
	// But we could use SteamGameServerNetworkingSockets() on Steam.
	m_pInterface = SteamNetworkingSockets();

	if (this->m_port <= 0 || this->m_port > 65535)
	{
		spdlog::error("Invalid port {}", this->m_port);
	}

	// Start listening
	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.ParseString(this->m_ip.toString().c_str());
	serverLocalAddr.m_port = this->m_port;

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
	m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
	if (m_hListenSock == k_HSteamListenSocket_Invalid)
		spdlog::error("Failed to listen on port %d", this->m_port);
	m_hPollGroup = m_pInterface->CreatePollGroup();
	if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
		spdlog::error("Failed to listen on port {}", this->m_port);
	spdlog::info("Server listening on port {}\n", this->m_port);
}

/// <summary>
/// Stops the server
/// </summary>
void Server::stop()
{
	spdlog::info("Closing connections...\n");
	for (auto it : m_mapClients)
	{
		// Send them one more goodbye message.  Note that we also have the
		// connection close reason as a place to send final data.  However,
		// that's usually best left for more diagnostic/debug text not actual
		// protocol strings.
		PDEPacket packet(PDEPacket::Disconnected);
		SendPacketToClient(it.first, packet);

		// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
		// to flush this out and close gracefully.
		m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
	}
	m_mapClients.clear();

	m_pInterface->CloseListenSocket(m_hListenSock);
	m_hListenSock = k_HSteamListenSocket_Invalid;

	m_pInterface->DestroyPollGroup(m_hPollGroup);
	m_hPollGroup = k_HSteamNetPollGroup_Invalid;
}

void Server::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	// What's the state of the connection?
	switch (pInfo->m_info.m_eState)
	{
		case k_ESteamNetworkingConnectionState_None:
			// NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		{

			// Ignore if they were not previously connected.  (If they disconnected
			// before we accepted the connection.)
			if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
			{
				// Locate the client.  Note that it should have been found, because this
				// is the only codepath where we remove clients (except on shutdown),
				// and connection change callbacks are dispatched in queue order.
				auto itClient = m_mapClients.find(pInfo->m_hConn);
				assert(itClient != m_mapClients.end());

				if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
				{
					spdlog::warn("Alas, {} hath fallen into shadow! (Internal Server Error)", itClient->second->name);
				}
				else
				{
					spdlog::warn("{} has disconnected! (Connection Closed by Peer)", itClient->second->name);
				}

				spdlog::info("Connection {}, reason: ",
					pInfo->m_info.m_szConnectionDescription,
					pInfo->m_info.m_eEndReason);

				if (this->m_world->getGameObject(itClient->second->id) != nullptr) {
					this->m_world->getGameObject(itClient->second->id)->destroySelf();
					this->m_world->getPlayers().erase(itClient->second);
				}

				// remove client from client map
				m_mapClients.erase(itClient);

				// TODO: possibly notify other clients?
			}
			else
			{
				assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
			}

			// Clean up the connection.  This is important!
			// The connection is "closed" in the network sense, but
			// it has not been destroyed.  We must close it on our end, too
			// to finish up.  The reason information do not matter in this case,
			// and we cannot linger because it's already closed on the other end,
			// so we just pass 0's.
			m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:
		{
			// This must be a new connection
			assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

			spdlog::info("Connection request from {}", pInfo->m_info.m_szConnectionDescription);

			// A client is attempting to connect
			// Try to accept the connection.
			if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
			{
				// This could fail.  If the remote host tried to connect, but then
				// disconnected, the connection may already be half closed.  Just
				// destroy whatever we have on our side.
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				spdlog::warn("Can't accept connection.  (It was already closed?)");
				break;
			}

			// Assign the poll group
			if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
			{
				m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
				spdlog::error("Failed to set poll group?");
				break;
			}

			// client connected
			// spawn new player on server
			std::string name = "Connecting...";
			std::shared_ptr<Connection> connection = std::make_shared<Connection>();
			connection->ip = pInfo->m_info.m_addrRemote;
			connection->id = Utility::generate_uuid_v4();
			connection->name = name;
			connection->port = pInfo->m_info.m_addrRemote.m_port;
			connection->state = PlayerState::ALIVE;

			std::string playerId = this->m_world->spawnPlayer(connection);

			std::string ip;
			connection->ip.ToString(ip.data(), ip.size(), false);
			spdlog::info("A player has connected: {}:{} - {}", ip, connection->port, connection->id);

			PDEPacket packet(PDEPacket::Connected);
			packet << connection->id;
			SendPacketToClient(pInfo->m_hConn, packet);

			this->m_mapClients.emplace(pInfo->m_hConn, connection);

			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:
			// We will get a callback immediately after accepting the connection.
			// Since we are the server, we can ignore this, it's not news to us.
			break;

		default:
			// Silences -Wswitch
			break;
	}
}

/// <summary>
/// Process: sends data to the clients, this should be called
/// on a fixed interval
/// </summary>
/// <param name="world">The game world</param>
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
	for (const auto& connPair : this->m_mapClients)
	{
		const auto conn = connPair.second;

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

		for (const auto& gameObjectPair : world->getGameObjects())
		{
			bool isNetworked = gameObjectPair.second->getNetworked();
			if (!isNetworked)
				continue;

			packet << true;
			if (gameObjectPair.second->getId() == "")
			{
				spdlog::error("Update packet is malformed!");
			}
			packet
				<< gameObjectPair.second->getType()
				<< gameObjectPair.second->getPosition().x
				<< gameObjectPair.second->getPosition().y
				<< gameObjectPair.second->getId()
				<< gameObjectPair.second->getHasBeenSent(conn->id);
			if (gameObjectPair.second->getHasBeenSent(conn->id)) {
				spdlog::debug("Sending existing object {} {}", gameObjectPair.second->getId(), gameObjectPair.second->getType());
				gameObjectPair.second->networkSerialize(packet);
			}
			else {
				spdlog::info("Sending new object {} {}", gameObjectPair.second->getId(), gameObjectPair.second->getType());
				gameObjectPair.second->networkSerializeInit(packet);
				gameObjectPair.second->setHasBeenSent(conn->id);
			}
		}
		packet << false;

		SendPacketToClient(connPair.first, packet);
	}
}

/// <summary>
/// Checks for new data from the clients
/// </summary>
/// <param name="world">The game world</param>
void Server::recieve(GameWorld* world)
{
	// poll connection state
	PollConnectionStateChanges();

	ISteamNetworkingMessage* pIncomingMsg = nullptr;
	int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
	if (numMsgs == 0)
		return;
	if (numMsgs < 0)
		spdlog::error("Error checking for messages");
	assert(numMsgs == 1 && pIncomingMsg);
	auto incomingClient = m_mapClients.find(pIncomingMsg->m_conn);
	assert(incomingClient != m_mapClients.end());

	PDEPacket incomingPkt;
	incomingPkt.clear();
	incomingPkt.onReceive(pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);

	// We don't need this anymore.
	pIncomingMsg->Release();


	// we now have a recieved packet in the normal format

	unsigned short numAxis = 0;
	unsigned short numButtons = 0;
	sf::Int8 idx = 0;
	sf::Int8 value = 0;
	bool valueBool = false;
	std::string clientId;
	std::string objId;
	bool isNetworked;

	// grab the client's id
	incomingPkt >> clientId;

	// get the player from the connection
	GameObject* player = world->getPlayer(incomingClient->second);

	
	std::shared_ptr<InputList> clientInputs = nullptr;

	// packet flag switch
	switch (incomingPkt.flag())
	{
		// we're recieving input values from the clients
		case PDEPacket::UserInput:
			// recieve input data
			clientInputs = this->m_inputManagers.at(incomingClient->second);

			incomingPkt >> numAxis >> numButtons;

			for (unsigned short i = 0; i < numAxis; i++)
			{
				incomingPkt >> idx >> value;
				for (const auto& input : clientInputs->inputs) {
					dynamic_cast<NetworkInputManager*>(input)->setAxis(idx, value);
				}

			}

			for (unsigned short i = 0; i < numButtons; i++)
			{
				incomingPkt >> idx >> valueBool;
				for (const auto& input : clientInputs->inputs) {
					dynamic_cast<NetworkInputManager*>(input)->setButton(idx, valueBool);
				}

			}

			float mouseX;
			float mouseY;
			incomingPkt >> mouseX >> mouseY;
			for (const auto& input : clientInputs->inputs) {
				dynamic_cast<NetworkInputManager*>(input)->setMouse(sf::Vector2f(mouseX, mouseY));
			}

			break;
	}
}

/// <summary>
/// Broadcasts health updates, makes
/// sure it's not being sent too frequently
/// </summary>
/// <param name="objName">the object id</param>
/// <param name="health">the new health</param>
void Server::broadcastObjectHealth(const std::string& objName, float health)
{
	// override timer for death packets
	if (this->m_broadcastCooldown.getElapsedTime().asMilliseconds() > 100 || health <= 0) {
		for (const auto& connPair : this->m_mapClients)
		{
			const auto& conn = connPair.second;

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
			SendPacketToClient(connPair.first, packet);
		}
		this->m_broadcastCooldown.restart();
	}
}

ConnectionStats::ConnectionStats()
{
	this->m_playerDeaths = 0;
	this->m_playerKills = 0;
}
