#pragma once
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <memory>

namespace PlatformDataEngine {

	class GameObject;

	class Component : public sf::Drawable
	{

	public:

		Component();

		// copy constructor
		Component(const Component& other);

		void registerHierarchy(GameObject* parent);

		virtual void init();

		virtual void update(const float& dt, const float& elapsedTime);

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		// must implement in any components so they can be loaded during gameObject
		// definition loading and parsing
		virtual void loadDefinition(nlohmann::json object) = 0;

	protected:
		std::map<std::string, nlohmann::json> m_properties;
		GameObject* m_parent;
	};
}

