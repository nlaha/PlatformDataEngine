#include "Client.h"

using namespace PlatformDataEngine;

Client::Client()
{
	spdlog::info("Running in CLIENT mode!");

	this->m_serverPort = 5660;
	this->m_serverIp = sf::IpAddress("localhost");
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
	PDEPacket packet;

	if (!this->m_isConnecting) {
		// try to connect to a server

		PDEPacket connectRequest(PDEPacket::Connect);
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
		// we're connected, start processing game packets
		const auto status = m_socket.receive(packet, this->m_serverIp, this->m_serverPort);

		switch (packet.flag())
		{
		case PDEPacket::SpawnGameObject:

			break;

		case PDEPacket::MoveGameObject:

			break;

		case PDEPacket::Connected:
			this->m_isConnected = true;
			spdlog::info("Client connected to server: {}:{}", m_serverIp.toString(), m_serverPort);

			break;

		default:
			break;
		}
	}
}
