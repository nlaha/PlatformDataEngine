#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <map>
#include <sstream>
#include "GameObject.h"
#include "Packet.h"
#include "GameWorld.h"
#include "Utility.h"
#include "NetworkHandler.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>

namespace PlatformDataEngine
{

	class ConnectionStats
	{

	public:
		ConnectionStats();

		sf::Uint16 m_playerDeaths;
		sf::Uint16 m_playerKills;

	private:
	};

	class Server : public NetworkHandler
	{
	public:
		Server(GameWorld *world = nullptr);

		void start();

		void stop();

		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo);

		void PollConnectionStateChanges();

		void process(GameWorld *world);

		void recieve(GameWorld *world);

		void destroyObject(std::string id);

		void replicateGameObject(GameObject* obj);
		void replicateGameObject(GameObject* obj, HSteamNetConnection conn);

		void broadcastObjectHealth(const std::string& objName, float health);

		inline void addInputManager(std::shared_ptr<Connection> ip, std::shared_ptr<InputManager> input)
		{
			if (this->m_inputManagers.count(ip) > 0)
			{
				this->m_inputManagers.find(ip)->second->inputs.push_back(input.get());
			}
			else
			{
				std::shared_ptr<InputList> inputList = std::make_shared<InputList>();
				inputList->inputs.push_back(input.get());
				this->m_inputManagers.emplace(ip, inputList);
			}
		}

		inline std::shared_ptr<Connection> getConnection() {
			return this->m_clientConnection;
		}

		// get connection from map by id
		inline std::shared_ptr<Connection> getConnection(const std::string &id)
		{
			for (const auto& connPair : this->m_mapClients)
			{
				if (connPair.second->id == id) {
					return connPair.second;
				}
			}
			return nullptr;
		}

	private:
		struct InputList
		{
			std::vector<InputManager *> inputs;
		};

		std::map<std::shared_ptr<Connection>, std::shared_ptr<InputList>> m_inputManagers;

		std::uint16_t m_port;
		sf::IpAddress m_ip;
		GameWorld *m_world;

		sf::Clock m_broadcastCooldown;
		std::map<std::shared_ptr<Connection>, ConnectionStats> m_connectionStats;

		/// <summary>
		/// STEAM
		/// </summary>

		HSteamListenSocket m_hListenSock;
		HSteamNetPollGroup m_hPollGroup;
		ISteamNetworkingSockets *m_pInterface;

		std::map<HSteamNetConnection, std::shared_ptr<Connection>> m_mapClients;

		/// <summary>
		/// Sends a packet to a client
		/// </summary>
		/// <param name="conn"></param>
		/// <param name="pkt"></param>
		void SendPacketToClient(HSteamNetConnection conn, PDEPacket &pkt)
		{
			size_t size = 0;
			const void *data = pkt.onSend(size);
			m_pInterface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
		}

		/// <summary>
		/// Sends a packet to all clients
		/// </summary>
		/// <param name="pkt"></param>
		/// <param name="except"></param>
		void SendPacketToAllClients(PDEPacket &pkt, HSteamNetConnection except = k_HSteamNetConnection_Invalid)
		{
			for (auto &c : m_mapClients)
			{
				if (c.first != except)
				{
					SendPacketToClient(c.first, pkt);
				}
			}
		}
	};
}
