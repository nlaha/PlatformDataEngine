#include "Component.h"

using namespace PlatformDataEngine;

PlatformDataEngine::Component::Component()
{
	this->m_parent = nullptr;
}

PlatformDataEngine::Component::Component(const Component& other)
{
	this->m_parent = other.m_parent;
	this->m_properties = other.m_properties;
}

void PlatformDataEngine::Component::registerHierarchy(GameObject* parent)
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
