#pragma once
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <map>
#include <sstream>
#include "GameObject.h"
#include "Packet.h"
#include "GameWorld.h"
#include "Utility.h"
#include "NetworkHandler.h"

namespace PlatformDataEngine {

	class ConnectionStats {

	public:
		ConnectionStats();
		
		sf::Uint16 m_playerDeaths;
		sf::Uint16 m_playerKills;

	private:


	};

	class Server : public NetworkHandler
	{
	public:

		Server();

		void start();

		void stop();

		void process(GameWorld* world);

		void recieve(GameWorld* world);

		void broadcastObjectHealth(const std::string& objName, float health);

		std::shared_ptr<Connection> findConnection(sf::IpAddress ip, std::string id);

		inline void addInputManager(std::shared_ptr<Connection> ip, std::shared_ptr<InputManager> input) {
			if (this->m_inputManagers.count(ip) > 0) {
				this->m_inputManagers.find(ip)->second->inputs.push_back(input.get());
			}
			else {
				std::shared_ptr<InputList> inputList = std::make_shared<InputList>();
				inputList->inputs.push_back(input.get());
				this->m_inputManagers.emplace(ip, inputList);
			}
		}

	private:

		struct InputList {
			std::vector<InputManager*> inputs;
		};

		std::map<std::shared_ptr<Connection>, std::shared_ptr<InputList>> m_inputManagers;
		std::vector<std::shared_ptr<Connection>> m_connections;

		unsigned short m_port;
		sf::IpAddress m_ip;
		sf::UdpSocket m_socket;

		sf::Clock m_broadcastCooldown;


		std::map<std::shared_ptr<Connection>, ConnectionStats> m_connectionStats;
	};
}
