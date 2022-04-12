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
	class Server : public NetworkHandler
	{
	public:
		Server();

		void start();

		void stop();

		void process(GameWorld* world);

	private:

		unsigned short m_port;
		sf::IpAddress m_ip;
		sf::UdpSocket m_socket;

	};
}
