#include "Server.h"

using namespace PlatformDataEngine;

Server::Server()
{
	spdlog::info("Running in SERVER mode!");

	this->m_port = 5660;
	this->m_ip = sf::IpAddress("localhost");
	this->m_socket.setBlocking(false);
}

void Server::start()
{
	// bind the m_socket to a port
	if (m_socket.bind(this->m_port, this->m_ip) != sf::Socket::Done)
	{
		spdlog::error("Could not bind socket to port {}", this->m_port);
	}
	else {
		spdlog::info("Server started, listening on port {}", this->m_port);
	}
}

void Server::stop()
{
}

void Server::process(GameWorld* world)
{
	PDEPacket packet;
	sf::IpAddress clientIp;
	unsigned short clientPort;

	m_socket.receive(packet, clientIp, clientPort);

	// player connected
	if (packet.flag() == PDEPacket::Connect) {
		spdlog::info("A player has connected with ip: {}", clientIp.toString());

		// spawn new player on server
		std::string playerId = world->spawnPlayer(clientIp.toString());

		// send connected message back
		PDEPacket connectedPacket(PDEPacket::Connected);
		connectedPacket << true;
		m_socket.send(connectedPacket, clientIp, clientPort);

		// send world data

		// bind player
	}
	else {
		switch (packet.flag())
		{
		case PDEPacket::UserInput:

			break;
			
		default:
			break;
		}
	}
}

