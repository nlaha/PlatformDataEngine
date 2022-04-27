#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <set>
#include <string>
#include <unordered_map>
#include "GameObject.h"
#include "Packet.h"
#include "GameWorld.h"
#include "NetworkHandler.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>

namespace PlatformDataEngine {
	class Client : public NetworkHandler
	{
	public:
		Client();

		void start();

		void stop();

		void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

		void PollConnectionStateChanges();

		void process(GameWorld* world);

		void recieve(GameWorld* world);

		void disconnect();

	private:

		HSteamNetConnection m_hConnection;
		ISteamNetworkingSockets* m_pInterface;

		std::set<std::string> m_objRequests;

		std::uint16_t m_serverPort;
		sf::IpAddress m_serverIp;

		bool m_isConnected;

		/// <summary>
		/// Sends a packet to the server
		/// reliably, this is kinda like a TCP packet
		/// but very fast. It's garunteed to get there eventually.
		/// </summary>
		/// <param name="conn"></param>
		/// <param name="pkt"></param>
		void SendPacketToServer(HSteamNetConnection conn, PDEPacket& pkt)
		{
			size_t size = 0;
			const void* data = pkt.onSend(size);
			m_pInterface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
		}

		/// <summary>
		/// Sends a packet to the server unreliably,
		/// this means low latency with the possibility of packet loss
		/// </summary>
		/// <param name="conn"></param>
		/// <param name="pkt"></param>
		void SendPacketToServerUnreliable(HSteamNetConnection conn, PDEPacket& pkt)
		{
			size_t size = 0;
			const void* data = pkt.onSend(size);
			m_pInterface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_UnreliableNoDelay, nullptr);
		}
	};
}
