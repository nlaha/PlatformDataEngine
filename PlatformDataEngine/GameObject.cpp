#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor, keep in mind this is called when creating a gameObject definition as well as an instance.
/// Definitions are loaded in memory as simple gameObjects so they can be instantiated using
/// the copy constructor and std::make_shared()
/// </summary>
GameObject::GameObject(bool isDef)
{
	// called for GameObjectDefinitions as well... 
	// even though they aren't actually added to the world!
	this->m_destroyed = false;
	this->m_zLayer = 0;
	this->m_isDefinition = isDef;
}

/// <summary>
/// Copy constructor
/// </summary>
/// <param name="other">other object</param>
GameObject::GameObject(const GameObject& other)
{
	for (const auto& componentPair : other.m_components)
	{
		std::string compType = componentPair.second->getType();
		std::shared_ptr<Component> compObj = ComponentFactory::create(compType);
		//nlohmann::json properties = componentPair.second->getProps();
		//compObj->loadDefinition(properties);
		compObj->copy(componentPair.second); // custom copy for special cases (physics body, etc.)
		compObj->registerHierarchy(this->m_self);

		this->m_components.emplace(compType + "%id%" + Utility::generate_uuid_v4(), compObj);
	}

	this->m_destroyed = other.m_destroyed;
	this->m_zLayer = other.m_zLayer;
	this->m_parent = other.m_parent;
	this->m_self = other.m_self;
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
	sortChildZ();
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

GameObject::~GameObject()
{
	this->m_components.clear();
	this->m_children.clear();
	this->m_parent = nullptr;
	this->m_self = nullptr;
}

/// <summary>
/// Drawcall, never call explicitly
/// </summary>
/// <param name="target"></param>
/// <param name="states"></param>
void GameObject::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// apply transform
	states.transform *= this->getTransform();

	if (this->m_components.size() > 0)
	{
		for (auto& compPair : this->m_components)
		{
			target.draw(*compPair.second, states);
		}
	}
	for (const auto& child : this->m_children)
	{
		child->draw(target, states);
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
		compObj->setIsDefinition(true);
		nlohmann::json properties = comp["properties"];
		compObj->loadDefinition(properties);
		compObj->setProps(properties);
		compObj->setType(compType);
		this->m_components.emplace(compType + "%id%" + Utility::generate_uuid_v4(), compObj);
	}
}

/// <summary>
/// Adds the gameObject pointer as the parent for each component this object owns
/// </summary>
/// <param name="self">a pointer to the new parent, usually "this" as a shared pointer</param>
void GameObject::registerComponentHierarchy(std::shared_ptr<GameObject> self)
{
	for (auto& compPair : this->m_components)
	{
		compPair.second->registerHierarchy(self.get());
	}
}


/// <summary>
/// Makes sure children are in the correct Z order for drawing
/// </summary>
void GameObject::sortChildZ()
{
	// z order sorting
	std::sort(this->m_children.begin(), this->m_children.end(), [](std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b) {
		return a->getZlayer() < b->getZlayer();
	});
}
