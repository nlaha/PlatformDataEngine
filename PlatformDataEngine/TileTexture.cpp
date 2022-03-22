#include "TileTexture.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
TileTexture::TileTexture(const sf::Texture* texture, sf::IntRect rect)
{
	this->m_rect = rect;
	this->m_texture = texture;
}

/// <summary>
/// Gets the rect.
/// </summary>
/// <returns>The rect.</returns>
const sf::IntRect& TileTexture::getRect() const
{
	return this->m_rect;
}


/// <summary>
/// Move the texture.
/// </summary>
/// <returns>The texture unique ptr.</returns>
const sf::Texture* TileTexture::getTexture()
{
	return this->m_texture;
}