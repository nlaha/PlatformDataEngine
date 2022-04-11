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

namespace PlatformDataEngine {

	/// <summary>
	/// The base container class for a gameplay related object, it's drawable, 
	/// transformable and components can be added to modify the funcionality. 
	/// These classes are defined by the user in a json file.
	/// </summary>
	class GameObject : public sf::Transformable, public sf::Drawable, public Alive
	{
	public:

		GameObject(bool isDef = false);
		~GameObject();

		// copy constructor
		GameObject(const GameObject& other);

		void init();
		void update(const float& dt, const float& elapsedTime);
		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void loadDefinition(std::string filename);
		void registerComponentHierarchy(std::shared_ptr<GameObject> self);

		inline std::string getName() const { return this->m_name; };
		inline bool isDefinition() const { return this->m_isDefinition; };
		inline void setIsDefinition(bool isDef) { this->m_isDefinition = isDef; };
		inline int getZlayer() const { return this->m_zLayer; };
		inline void setZlayer(int zLayer) { this->m_zLayer = zLayer; };
		inline void addChild(std::shared_ptr<GameObject> gameObject) { this->m_children.push_back(gameObject); };
		inline void setParent(std::shared_ptr<GameObject> gameObject) { this->m_parent = gameObject; };
		inline void setIsUI(bool isUI) { this->m_isUI = isUI; };
		inline bool getIsUI() const { return this->m_isUI; };
		inline std::shared_ptr<GameObject> getParent() { return this->m_parent; };

		inline void setName(std::string& name) { this->m_name = name; };
		inline void destroySelf() {
			this->m_destroyed = true;
			for (std::shared_ptr<GameObject> child : this->m_children)
			{
				child->destroySelf();
			}
		};

		inline bool getDestroyed() const { return this->m_destroyed; };
		inline std::vector<std::shared_ptr<GameObject>> getChildren() const { return this->m_children; };

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

		std::string m_id;
		std::string m_name;
		int m_zLayer;
		bool m_destroyed;
		bool m_isDefinition;
		bool m_isUI;

		bool m_hasHealthBar;
		std::shared_ptr<StatsBar> m_healthBar;

		std::map<std::string, nlohmann::json> m_properties;
	};
}