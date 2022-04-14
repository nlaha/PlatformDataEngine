#pragma once
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include "Packet.h"
#include "Networkable.h"

namespace PlatformDataEngine {

	class GameObject;

	/// <summary>
	/// The base component class, when creating components to
	/// add gameplay functionality, they should inherit from this class
	/// </summary>
	class Component : public sf::Drawable, public Networkable
	{

	public:

		Component();

		virtual void copy(std::shared_ptr<Component> otherCompPtr);

		void registerHierarchy(GameObject* parent);

		virtual void init();

		virtual void update(const float& dt, const float& elapsedTime);

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		virtual void networkSerializeInit(PDEPacket& output);
		virtual void networkDeserializeInit(PDEPacket& input);

		virtual void networkSerialize(PDEPacket& output);
		virtual void networkDeserialize(PDEPacket& input);

		// must implement in any components so they can be loaded during gameObject
		// definition loading and parsing
		virtual void loadDefinition(nlohmann::json object);

		std::string getType() const { return this->m_type; };
		nlohmann::json getProps() const { return this->m_all_props; };

		void setType(std::string type) { this->m_type = type;  };
		void setProps(nlohmann::json& props) { this->m_all_props = props; };

		inline void setIsDefinition(bool isDef) { this->m_isDefinition = isDef; };

	protected:
		std::map<std::string, nlohmann::json> m_properties;
		GameObject* m_parent;
		bool m_isDefinition;

		nlohmann::json m_all_props;
		std::string m_type;
	};
}

