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
	this->m_isUI = false;
	this->m_self = nullptr;
	this->m_hasHealthBar = false;
	this->m_healthBar = nullptr;
	this->m_id = "";
	this->m_owningConnection = nullptr;
	this->m_type = "";
	this->m_hasPhysics = true;
	this->m_networked = true;
}

/// <summary>
/// Copy constructor
/// </summary>
/// <param name="other">other object</param>
GameObject::GameObject(const GameObject &other)
{
	for (const auto &componentPair : other.m_components)
	{
		std::string compType = componentPair.second->getType();
		std::shared_ptr<Component> compObj = ComponentFactory::create(compType);
		// nlohmann::json properties = componentPair.second->getProps();
		// compObj->loadDefinition(properties);
		compObj->copy(componentPair.second); // custom copy for special cases (physics body, etc.)
		compObj->registerHierarchy(this->m_self);

		this->m_components.emplace(compType + "%id%" + Utility::generate_uuid_v4(), compObj);
	}

	this->m_destroyed = other.m_destroyed;
	this->m_zLayer = other.m_zLayer;
	this->m_parent = other.m_parent;
	this->m_self = nullptr;
	this->m_id = other.m_id;
	this->m_type = other.m_type;
	this->m_objName = other.m_objName;
	this->m_properties = other.m_properties;
	this->m_HP = 100.0f;

	if (other.m_healthBar != nullptr)
		this->m_healthBar = std::make_shared<StatsBar>(*other.m_healthBar);
	this->m_hasHealthBar = other.m_hasHealthBar;
	this->m_alreadyReplicated = false;
}

/// <summary>
/// Calls init() for every component in this gameObject and itself
/// </summary>
void GameObject::init()
{
	this->m_children.resize(0);

	this->m_hasPhysics = this->findComponentOfType<PhysicsBody>() != nullptr;

	if (this->m_hasHealthBar && this->m_healthBar != nullptr)
	{
		this->m_healthBar->init();
	}

	for (auto &compPair : this->m_components)
	{
		compPair.second->init();
	}
	sortChildZ();

	this->m_nameText.setText(this->m_objName);
	this->m_nameText.setScale({0.05f, 0.05f});
}

/// <summary>
/// Runs the update loop for every component in this gameObject and itself
/// </summary>
/// <param name="dt">delta time</param>
/// <param name="elapsedTime">elapsed time (since game started)</param>
void GameObject::update(const float &dt, const float &elapsedTime)
{
	if (this->m_hasHealthBar && this->m_healthBar != nullptr)
	{
		this->m_healthBar->setPosition(Utility::fromB2(this->findComponentOfType<PhysicsBody>()->getBody()->GetWorldCenter()));
		this->m_healthBar->update(dt, elapsedTime, this->m_HP);
	}

	for (auto &compPair : this->m_components)
	{
		compPair.second->update(dt, elapsedTime);
	}

	if (this->m_objName != "")
	{
		this->m_nameText.setText(fmt::format("{}", this->m_objName));
	}
}

void GameObject::networkSerialize(PDEPacket &output)
{
	// serialize components
	for (auto &compPair : this->m_components)
	{
		compPair.second->networkSerialize(output);
	}
}

void GameObject::networkDeserialize(PDEPacket &input)
{
	// update components
	for (auto& compPair : this->m_components)
	{
		compPair.second->networkDeserialize(input);
	}

	//// update child links
	//if (this->m_children.size() < this->m_childNames.size())
	//{
	//	for (const std::string &name : this->m_childNames)
	//	{
	//		if (PlatformDataEngineWrapper::getWorld()->getGameObject(name) != nullptr)
	//		{
	//			this->addChild(PlatformDataEngineWrapper::getWorld()->getGameObject(name));
	//			PlatformDataEngineWrapper::getWorld()->getGameObject(name)->setParent(
	//				PlatformDataEngineWrapper::getWorld()->getGameObject(this->getId()));
	//		}
	//	}
	//}
}

void GameObject::networkSerializeInit(PDEPacket &output)
{
	//output << this->getPosition().x << this->getPosition().y;

	//output << this->m_zLayer << this->m_isUI;

	//output << this->getOrigin().x << this->getOrigin().y;

	//output << this->m_children.size();
	//for (std::shared_ptr<GameObject> child : this->m_children)
	//{
	//	output << child->getId();
	//}

	//output << this->m_objName;
}

void GameObject::networkDeserializeInit(PDEPacket &input)
{
	spdlog::info("Deserialize Init for {} - {}", this->m_id, this->m_type);
	
	//sf::Vector2f pos;
	//input >> pos.x >> pos.y;

	//this->setPosition(pos);

	//input >> this->m_zLayer >> this->m_isUI;

	//float ox = 0.0f, oy = 0.0f;
	//input >> ox >> oy;
	//this->setOrigin(ox, oy);

	//size_t numChildren = 0;
	//input >> numChildren;
	//spdlog::info("child size: {}", numChildren);
	//for (size_t i = 0; i < numChildren; i++)
	//{
	//	std::string name;
	//	input >> name;
	//	this->m_childNames.push_back(name);
	//}

	//input >> this->m_objName;
	//this->m_nameText.setText(this->m_objName);
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
void GameObject::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	// apply transform
	sf::Transform ourTransform = this->getTransform();
	if (this->m_isUI)
	{
		sf::Vector2f uiOffset = PlatformDataEngineWrapper::getWorld()->getCameraController().getUIOffset();
		ourTransform.translate(uiOffset);
	}
	states.transform *= ourTransform;

	if (this->m_components.size() > 0)
	{
		for (auto &compPair : this->m_components)
		{
			target.draw(*compPair.second, states);
		}
	}
	for (const auto &child : this->m_children)
	{
		child->draw(target, states);
	}

	// draw health bar if we have that enabled
	if (this->m_hasHealthBar && this->m_healthBar != nullptr)
	{
		target.draw(*this->m_healthBar, states);
	}

	if (this->m_nameText.getText() != "")
	{
		target.draw(m_nameText, states);
	}
}

/// <summary>
/// Loads a gameObject definition from a json file
/// </summary>
/// <param name="filename">game object definition .json filename</param>
void GameObject::loadDefinition(const std::string &filename)
{
	// load json file
	std::ifstream file(filename);

	std::string base_filename = filename.substr(filename.find_last_of("/\\") + 1);
	std::string::size_type const p(base_filename.find_last_of('.'));
	std::string file_without_extension = base_filename.substr(0, p);
	this->m_type = file_without_extension;

	// parse json file
	nlohmann::json object;
	file >> object;

	this->m_isUI = false;

	if (object.count("networked") > 0)
	{
		this->m_networked = object.at("networked");
	}

	if (object.count("hasHealthBar") > 0)
		this->m_hasHealthBar = object.at("hasHealthBar");
	if (this->m_hasHealthBar)
		this->m_healthBar = std::make_shared<StatsBar>();
	if (object.count("healthBar") > 0)
		this->m_healthBar->loadDefinition(object.at("healthBar"));

	// load components
	for (auto &comp : object["components"])
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
	this->m_self = self.get();
	for (auto &compPair : this->m_components)
	{
		compPair.second->registerHierarchy(self.get());
	}
}

void PlatformDataEngine::GameObject::destroySelf()
{
	this->m_destroyed = true;
	if (!PlatformDataEngineWrapper::getIsClient()) {
		for (std::shared_ptr<GameObject> child : this->m_children)
		{
			child->destroySelf();
		}
	}
}

/// <summary>
/// Makes sure children are in the correct Z order for drawing
/// </summary>
void GameObject::sortChildZ()
{
	// z order sorting
	std::sort(this->m_children.begin(), this->m_children.end(), [](std::shared_ptr<GameObject> a, std::shared_ptr<GameObject> b)
			  { return a->getZlayer() < b->getZlayer(); });
}

void GameObject::onDeath()
{
	std::shared_ptr<DamageHandler> dh = this->findComponentOfType<DamageHandler>();
	if (dh != nullptr)
		dh->onDeath();

	if (!PlatformDataEngineWrapper::getIsClient())
	{
		// player has died
		// cast to server
		Server* server = reinterpret_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler());

		if (server->getConnection(this->getId()) != nullptr) {
			server->getConnection(this->getId())->state = PlayerState::DEAD;
			server->getConnection(this->getId())->respawnTimer.restart();
			spdlog::info("Player {} has died!", server->getConnection()->name);
		}
		else {
			if (this->getId() == PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->id)
			{
				PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->state = PlayerState::DEAD;
				PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->respawnTimer.restart();
			}
		}
	}
	else {
		if (this->getId() == PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->id)
		{
			PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->state = PlayerState::DEAD;
			PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->respawnTimer.restart();
		}
	}

	this->destroySelf();
}

void GameObject::onDamage(float currentHP)
{
	if (this->m_hasHealthBar)
	{
		std::shared_ptr<DamageHandler> dh = this->findComponentOfType<DamageHandler>();
		if (dh != nullptr)
			dh->onDamage(currentHP);

		 if (!PlatformDataEngineWrapper::getIsClient()) {
			dynamic_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler())->
				broadcastObjectHealth(this->m_id, this->m_HP);
		 }
	}
	else
	{
		this->m_HP = 100.0f;
	}
}
