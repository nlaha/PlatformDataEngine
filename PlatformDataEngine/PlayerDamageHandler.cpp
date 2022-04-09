#include "PlayerDamageHandler.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

void PlayerDamageHandler::init()
{
}

void PlayerDamageHandler::update(const float& dt, const float& elapsedTime)
{
}

void PlayerDamageHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PlayerDamageHandler::copy(std::shared_ptr<Component> otherCompPtr)
{
}

void PlayerDamageHandler::loadDefinition(nlohmann::json object)
{
}

void PlayerDamageHandler::onDeath()
{
	this->m_parent->destroySelf();
}

void PlayerDamageHandler::onDamage(float currentHP)
{
}
