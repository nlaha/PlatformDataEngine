#pragma once
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

#include "Utility.h"
#include "Component.h"
#include "ComponentFactory.h"

namespace PlatformDataEngine {

	class GameObject : public sf::Transformable, public sf::Drawable
	{
	public:

		GameObject();
		~GameObject();

		// copy constructor
		GameObject(const GameObject& other);

		void init();
		void update(const float& dt, const float& elapsedTime);
		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void loadDefinition(std::string filename);
		void registerComponentHierarchy(std::shared_ptr<GameObject> self);

		template<typename T>
		inline std::shared_ptr<T> findComponentOfType()
		{
			for (auto& compPair : this->m_components)
			{
				// check if T is equal to the type of the component
				if (typeid(T) == typeid(*compPair.second))
				{
					std::shared_ptr<T> derived =
						std::dynamic_pointer_cast<T> (compPair.second);
					return derived;
				}
			}
		}

	private:
		std::map<std::string, std::shared_ptr<Component>> m_components;
		std::string m_id;
		std::string m_name;

		std::map<std::string, nlohmann::json> m_properties;
	};
}