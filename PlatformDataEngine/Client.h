#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include "MessageTypes.h"
#include "GameObject.h"
#include "Packet.h"
#include "GameWorld.h"
#include "NetworkHandler.h"

namespace PlatformDataEngine {
	class Client : public NetworkHandler
	{
	public:
		Client();

		void start();

		void stop();

		void process(GameWorld* world);

		void recieve(GameWorld* world);

		void disconnect();

		inline std::shared_ptr<Connection> getConnection() { return this->m_clientConnection; };

	private:

		std::shared_ptr<Connection> m_clientConnection;

		unsigned short m_serverPort;
		sf::IpAddress m_serverIp;
		sf::UdpSocket m_socket;
		bool m_isConnecting;
		bool m_isConnected;

	};
}
