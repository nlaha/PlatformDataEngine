#include "GameObject.h"

using namespace PlatformDataEngine;

GameObject::GameObject()
{
	// called for GameObjectDefinitions as well... 
	// even though they aren't actually added to the world!
}

GameObject::GameObject(const GameObject& other)
{
	this->m_components = other.m_components;
	this->m_name = other.m_name;
	this->m_id = other.m_id;
	this->m_properties = other.m_properties;
}

void GameObject::init()
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->init();
	}
}

void GameObject::update(const float& dt, const float& elapsedTime)
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->update(dt, elapsedTime);
	}
}

PlatformDataEngine::GameObject::~GameObject()
{

}

void GameObject::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// apply transform
	states.transform *= getTransform();
	if (this->m_components.size() > 0)
	{
		for (auto& compPair : this->m_components)
		{
			target.draw(*compPair.second, states);
		}
	}
}

void GameObject::loadDefinition(std::string filename) {
	// load json file
	std::ifstream file(filename);
	
	// parse json file
	nlohmann::json object;
	file >> object;
	
	// load components
	for (auto& comp : object["components"])
	{
		std::string compType = comp["type"];
		std::shared_ptr<Component> compObj = ComponentFactory::create(compType);
		nlohmann::json properties = comp["properties"];
		compObj->loadDefinition(properties);
		this->m_components.emplace(compType + "%id%" + Utility::generate_uuid_v4(), compObj);
	}
}

void PlatformDataEngine::GameObject::registerComponentHierarchy(std::shared_ptr<GameObject> self)
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->registerHierarchy(self.get());
	}
}
