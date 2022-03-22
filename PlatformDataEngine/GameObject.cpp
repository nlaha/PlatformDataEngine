#include "GameObject.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor, keep in mind this is called when creating a gameObject definition as well as an instance.
/// Definitions are loaded in memory as simple gameObjects so they can be instantiated using
/// the copy constructor and std::make_shared()
/// </summary>
GameObject::GameObject()
{
	// called for GameObjectDefinitions as well... 
	// even though they aren't actually added to the world!
}

/// <summary>
/// Copy constructor
/// </summary>
/// <param name="other">other object</param>
GameObject::GameObject(const GameObject& other)
{
	this->m_components = other.m_components;
	this->m_name = other.m_name;
	this->m_id = other.m_id;
	this->m_properties = other.m_properties;
}

/// <summary>
/// Calls init() for every component in this gameObject and itself
/// </summary>
void GameObject::init()
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->init();
	}
}

/// <summary>
/// Runs the update loop for every component in this gameObject and itself
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time (since game started)</param>
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

/// <summary>
/// Drawcall, never call explicitly
/// </summary>
/// <param name="target"></param>
/// <param name="states"></param>
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

/// <summary>
/// Loads a gameObject definition from a json file
/// </summary>
/// <param name="filename">game object definition .json filename</param>
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

/// <summary>
/// Adds the gameObject pointer as the parent for each component this object owns
/// </summary>
/// <param name="self">a pointer to the new parent, usually "this" as a shared pointer</param>
void PlatformDataEngine::GameObject::registerComponentHierarchy(std::shared_ptr<GameObject> self)
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->registerHierarchy(self.get());
	}
}
