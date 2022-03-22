#pragma once
#include <memory>
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine {

	class TileTexture
	{
	public:
		TileTexture(const sf::Texture* texture = nullptr, sf::IntRect rect = sf::IntRect());

		const sf::IntRect& getRect() const;
		const sf::Texture* getTexture();

	private:
		const sf::Texture* m_texture;
		sf::IntRect m_rect;
	};
}