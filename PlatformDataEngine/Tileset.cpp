#include "Tileset.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
Tileset::Tileset(const std::string& imagePath, int tileSize, const std::string& shaderPath)
{
	this->m_tileSize = tileSize;
	this->m_shader = std::make_shared<sf::Shader>();
	loadTileset(imagePath, shaderPath);
}


/// <summary>
/// Loads the tileset.
/// </summary>
/// <returns>true if successful, false if not</returns>
bool Tileset::loadTileset(const std::string& imagePath, const std::string& shaderPath)
{
	// load tileset texture
	this->m_texture = std::make_unique<sf::Texture>();
	if (!this->m_texture->loadFromFile(imagePath))
	{
		spdlog::error("Could not load image: {}", imagePath);
		return false;
	}
	spdlog::info("Loaded image: {}", imagePath);
	this->m_texture->setSmooth(false);
	this->m_texture->setRepeated(true);

	// load shader if present
	if (shaderPath != "") {
		if (!this->m_shader->loadFromFile(shaderPath + ".vert", shaderPath + ".frag"))
		{
			spdlog::error("Could not load shader: {}", shaderPath);
			return false;
		}
		spdlog::info("Loaded shader: {}", shaderPath);
	}

	sf::Vector2u texSize = this->m_texture->getSize();

	// split texture into tiles and load each tile into the vector
	for (size_t y = 0; y < texSize.y / this->m_tileSize; y++)
	{
		for (size_t x = 0; x < texSize.x / this->m_tileSize; x++)
		{
			TileTexture tex(this->m_texture.get(), sf::IntRect(
				x, 
				y, 
				this->m_tileSize,
				this->m_tileSize
			));
			this->m_tileTextures.push_back(tex);
		}
	}

	return true;
}

void Tileset::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	sf::Vector2i viewCenter(target.getView().getCenter());
	sf::Vector2i viewSize(target.getView().getSize());
	sf::FloatRect currentViewRect
	(viewCenter.x - viewSize.x / 2, // left
		viewCenter.y - viewSize.y / 2, // top
		viewSize.x,
		viewSize.y);

	// loop through tile sprites and draw each one using a vertex array
	sf::VertexArray quads(sf::Quads);

	sf::Vector2u texSize = this->m_texture->getSize();
	auto lambdaAddQuad = [&quads, &texSize, &currentViewRect](const TileTexture tex, const sf::Transform& transform, const int tileSize) {
		sf::Vertex quad[4];

		sf::FloatRect rect = { {0, 0}, {(float)tileSize, (float)tileSize} };
		rect = transform.transformRect(rect);

		// convert pixel coordinates to uv coordinates
		int tu = tex.getRect().left;
		int tv = tex.getRect().top;

		rect.left -= 0.1f;
		rect.top -= 0.1f;
		rect.height += 0.1f;
		rect.width += 0.1f;

		if (rect.intersects(currentViewRect))
		{
			// define its 4 corners
			quad[0].position = sf::Vector2f(rect.left, rect.top);
			quad[1].position = sf::Vector2f(rect.left + rect.width, rect.top);
			quad[2].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
			quad[3].position = sf::Vector2f(rect.left, rect.top + rect.height);

			// define its 4 texture coordinates
			quad[0].texCoords = sf::Vector2f(tu * tileSize, tv * tileSize);
			quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize, tv * tileSize);
			quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize, (tv + 1) * tileSize);
			quad[3].texCoords = sf::Vector2f(tu * tileSize, (tv + 1) * tileSize);

			quads.append(quad[0]);
			quads.append(quad[1]);
			quads.append(quad[2]);
			quads.append(quad[3]);
		}
	};

	std::for_each(
		std::execution::seq,
		this->m_tileSprites.begin(),
		this->m_tileSprites.end(),
		[&](TileSprite* sprite)
		{
			sprite->preDraw();

			// add the sprite as a quad to be rendered
			lambdaAddQuad(
				this->getTileTexture(sprite->getTileIdx()),
				sprite->getTransform(),
				this->m_tileSize
			);
		});

	states.texture = this->m_texture.get();

	states.shader = this->m_shader.get();

	target.draw(quads, states);
}

void Tileset::update(const float& dt, const float& elapsedTime)
{
	this->m_shader->setUniform("time", elapsedTime);
}


/// <summary>
/// Gets the tile texture.
/// </summary>
/// <returns>The tile texture.</returns>
const TileTexture& Tileset::getTileTexture(int tileIdx) const
{
	return this->m_tileTextures.at(tileIdx);
}

void Tileset::registerSprite(TileSprite* sprite)
{
	this->m_tileSprites.push_back(sprite);
}

void Tileset::deregisterSprite(TileSprite* sprite)
{
	// remove sprite from draw vector
	this->m_tileSprites.erase(std::remove(
		this->m_tileSprites.begin(), 
		this->m_tileSprites.end(), 
		sprite
	), this->m_tileSprites.end());
}
