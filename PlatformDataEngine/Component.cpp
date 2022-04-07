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

/// <summary>
/// Essentially a copy constructor except the generic shared pointer
/// must be converted to the derived type using std::dynamic_pointer_cast
/// </summary>
/// <param name="otherCompPtr"></param>
void Component::copy(std::shared_ptr<Component> otherCompPtr)
{
	*this = *otherCompPtr;
}

void Component::registerHierarchy(GameObject* parent)
{
	this->m_parent = parent;
}

/// <summary>
/// Called once on startup
/// </summary>
void Component::init()
{
}

/// <summary>
/// Called every update cycle
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time</param>
void Component::update(const float& dt, const float& elapsedTime)
{
}

/// <summary>
/// Called every draw cycle
/// </summary>
/// <param name="target"></param>
/// <param name="states"></param>
void Component::draw(sf::RenderTarget& target, sf::RenderStates states) const
{

}

/// <summary>
/// Called when game object definitions are loaded
/// </summary>
/// <param name="object"></param>
void Component::loadDefinition(nlohmann::json object)
{
}
