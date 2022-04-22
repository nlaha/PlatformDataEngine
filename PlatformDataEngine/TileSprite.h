#pragma once

#include <memory>
#include <box2d/box2d.h>

#include "Tileset.h"
#include "Globals.h"

namespace PlatformDataEngine {

	class Tileset;

	/// <summary>
	/// Since tilesets are drawn each in a single draw call, this isn't 
	/// responsible for holding a texture or something like a normal sprite would hold.
	/// Instead it holds a tile index and a transform that can be referenced in a tileset
	/// </summary>
	class TileSprite : public sf::Transformable
	{
	public:
		TileSprite(std::shared_ptr<Tileset> tileset, b2PolygonShape& collisionShape, int tileIdx = 0, bool hasPhysics = false);
		TileSprite(std::shared_ptr<Tileset> tileset, b2PolygonShape& collisionShape, int tileIdx = 0, bool hasPhysics = false, sf::Vector2f position = { 0, 0 });

		~TileSprite();

		// copy constructor
		TileSprite(const TileSprite& other);

		inline int getTileIdx() const { return this->m_tileIdx; };

		inline void preDraw() {};

	private:
		void registerSelf(sf::Vector2f pos = {0.0, 0.0});
		virtual void initTransform();

		std::shared_ptr<Tileset> m_tileset;
		int m_tileIdx;

		b2Body* m_body;
		std::shared_ptr<b2PolygonShape> m_bodyShape;

		bool m_hasPhysics;
	};
}

