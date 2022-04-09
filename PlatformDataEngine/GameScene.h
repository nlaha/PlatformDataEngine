#pragma once
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine {

	class GameObject;

	class GameScene
	{
	public:
		virtual void init() = 0;

		virtual void update(const float& dt, const float& elapsedTime) = 0;

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

	protected:
		std::map<std::string, std::shared_ptr<GameObject>> mp_gameObjects;
	};
}

