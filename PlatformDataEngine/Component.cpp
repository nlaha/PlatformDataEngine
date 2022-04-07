#include "Component.h"

using namespace PlatformDataEngine;

Component::Component()
{
	this->m_parent = nullptr;
	this->m_isDefinition = false;
}

Component::~Component()
{
}

void Component::copy(std::shared_ptr<Component> otherCompPtr)
{
	*this = *otherCompPtr;
}

void Component::registerHierarchy(GameObject* parent)
{
	this->m_parent = parent;
}

void Component::init()
{
}

void Component::update(const float& dt, const float& elapsedTime)
{
}

void Component::draw(sf::RenderTarget& target, sf::RenderStates states) const
{

}

void Component::loadDefinition(nlohmann::json object)
{
}
