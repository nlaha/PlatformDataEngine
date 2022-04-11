#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine {
	
	class GameObject;
	class SpriteRenderer;

	class GlobalEffects
	{
	public:

		static void explode(SpriteRenderer& sprite, sf::Vector2f pos, int slices = 8);
	};
}

