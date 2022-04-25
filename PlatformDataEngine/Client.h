#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
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

		std::uint16_t m_serverPort;
		sf::IpAddress m_serverIp;

		void SendPacketToServer(HSteamNetConnection conn, PDEPacket& pkt)
		{
			size_t size = 0;
			const void* data = pkt.onSend(size);
			m_pInterface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
		}

		void SendPacketToServerUnreliable(HSteamNetConnection conn, PDEPacket& pkt)
		{
			size_t size = 0;
			const void* data = pkt.onSend(size);
			m_pInterface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_Unreliable, nullptr);
		}
	};
}
