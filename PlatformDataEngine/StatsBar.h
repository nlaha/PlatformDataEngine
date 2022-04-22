#pragma once
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

#include "Utility.h"
#include "Packet.h"

namespace PlatformDataEngine {
	
	class StatsBar : public sf::Drawable
	{
	public:
		void init();

		void update(const float& dt, const float& elapsedTime, float hp);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void loadDefinition(nlohmann::json object);

		void setPosition(sf::Vector2f pos);

		void networkSerialize(PDEPacket& output);
		void networkDeserialize(PDEPacket& input);

	private:
		sf::RectangleShape m_healthBar;
		sf::RectangleShape m_healthBarDelayed;
		sf::RectangleShape m_healthBarBackground;

		float m_health;
		float m_targetHealth;
		float m_healthBarSpeed;

		sf::Vector2f m_healthBarSize;

		sf::Vector2f m_position;
	};
}

