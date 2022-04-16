#include "Utility.h"

using namespace::PlatformDataEngine;

void Connection::networkSerialize(PDEPacket& output)
{
	switch (this->state)
	{
	case PlayerState::ALIVE:
		output << sf::Uint8(0);
		break;

	case PlayerState::DEAD:
		output << sf::Uint8(1);
		break;

	case PlayerState::SPECTATOR:
		output << sf::Uint8(2);
		break;

	}

	output << this->name;
	output << this->health;
}

void Connection::networkDeserialize(PDEPacket& input)
{
	sf::Uint8 inputState = 0;
	input >> inputState;
	
	switch (inputState)
	{
	case sf::Uint8(0):
		this->state = PlayerState::ALIVE;
		break;
	case sf::Uint8(1):
		this->state = PlayerState::DEAD;
		break;
	case sf::Uint8(2):
		this->state = PlayerState::SPECTATOR;
		break;
	}

	input >> this->name;
	input >> this->health;
}
