#pragma once
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <typeinfo>

#include "Utility.h"
#include "Component.h"
#include "Alive.h"
#include "ComponentFactory.h"
#include "StatsBar.h"
#include "Packet.h"

namespace PlatformDataEngine {

	/// <summary>
	/// The base container class for a gameplay related object, it's drawable, 
	/// transformable and components can be added to modify the funcionality. 
	/// These classes are defined by the user in a json file.
	/// </summary>
	class GameObject : public sf::Transformable, public sf::Drawable, public Alive, public Networkable
	{
	public:

		GameObject(bool isDef = false);
		~GameObject();

		// copy constructor
		GameObject(const GameObject& other);

		void init();
		void update(const float& dt, const float& elapsedTime);
		void networkSerialize(PDEPacket& output);
		void networkDeserialize(PDEPacket& input);
		void networkSerializeInit(PDEPacket& output);
		void networkDeserializeInit(PDEPacket& input);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void loadDefinition(const std::string& filename);
		void registerComponentHierarchy(std::shared_ptr<GameObject> self);

		inline bool isDefinition() const { return this->m_isDefinition; };
		inline void setIsDefinition(bool isDef) { this->m_isDefinition = isDef; };
		inline int getZlayer() const { return this->m_zLayer; };
		inline void setZlayer(int zLayer) { this->m_zLayer = zLayer; };
		inline void addChild(std::shared_ptr<GameObject> gameObject) { this->m_children.push_back(gameObject); };
		inline void setParent(std::shared_ptr<GameObject> gameObject) { this->m_parent = gameObject; };
		inline void setIsUI(bool isUI) { this->m_isUI = isUI; };
		inline bool getIsUI() const { return this->m_isUI; };
		inline void setConnection(std::shared_ptr<Connection> conn) { this->m_owningConnection = conn; };
		inline std::shared_ptr<Connection> getConnection() const { return this->m_owningConnection; };
		inline std::shared_ptr<GameObject> getParent() { return this->m_parent; };
		inline void setAlreadyReplicated(bool replicated) { this->m_alreadyReplicated = replicated; };

		inline std::string getId() const { return this->m_id; };
		inline void setId(const std::string& id) { this->m_id = id; };

		inline void setName(const std::string& name) { this->m_objName = name; };

		inline void destroySelf() {
			this->m_destroyed = true;
			for (std::shared_ptr<GameObject> child : this->m_children)
			{
				child->destroySelf();
			}
		};

		inline std::string getType() const { return this->m_type; };
		inline void setType(const std::string& type) { this->m_type = type; };
		inline bool getDestroyed() const { return this->m_destroyed; };
		inline std::vector<std::shared_ptr<GameObject>> getChildren() const { return this->m_children; };
		inline void setNetworked(bool networked) { this->m_networked = networked; };
		inline bool getNetworked() { return this->m_networked; };

		inline void setHasBeenSent(const std::string& id) { this->m_hasBeenSent.emplace(id, true); };
		inline bool getHasBeenSent(const std::string& id) const {
			if (!this->m_alreadyReplicated) {
				return this->m_hasBeenSent.count(id) > 0;
			}
			else {
				return true;
			}
		};

		void sortChildZ();

		template<typename T>
		inline std::shared_ptr<T> findComponentOfType() const
		{
			for (auto& compPair : this->m_components)
			{
				// check if T is equal to the type of the component
				if (std::dynamic_pointer_cast<T>(compPair.second))
				{
					std::shared_ptr<T> derived =
						std::dynamic_pointer_cast<T> (compPair.second);
					return derived;
				}
			}
			return nullptr;
		}

		void onDeath();
		void onDamage(float currentHP);

	private:

		std::map<std::string, std::shared_ptr<Component>> m_components;
		std::vector<std::shared_ptr<GameObject>> m_children;
		std::shared_ptr<GameObject> m_parent;
		GameObject* m_self;

		std::shared_ptr<Connection> m_owningConnection;

		std::string m_objName;
		std::string m_id;
		int m_zLayer;
		bool m_alreadyReplicated;
		bool m_destroyed;
		bool m_isDefinition;
		bool m_isUI;
		bool m_networked;
		std::map<std::string, bool> m_hasBeenSent;
		std::string m_type;

		bool m_hasPhysics;

		TextDrawable m_nameText;

		std::vector<std::string> m_childNames;

		bool m_hasHealthBar;
		std::shared_ptr<StatsBar> m_healthBar;

		std::map<std::string, nlohmann::json> m_properties;
	};
}