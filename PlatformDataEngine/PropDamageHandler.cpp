#include "PropDamageHandler.h"
#include "GameObject.h"
#include "GlobalEffects.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

void PropDamageHandler::init()
{
}

void PropDamageHandler::update(const float& dt, const float& elapsedTime)
{
}

void PropDamageHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PropDamageHandler::copy(std::shared_ptr<Component> otherCompPtr)
{
}

void PropDamageHandler::loadDefinition(nlohmann::json object)
{
}

void PropDamageHandler::onDeath()
{
	GlobalEffects::explode(*this->m_parent->findComponentOfType<SpriteRenderer>(), this->m_parent->getPosition());

	this->m_parent->destroySelf();
}

void PropDamageHandler::onDamage(float currentHP)
{
}
