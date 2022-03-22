#pragma once

#include <memory>
#include <box2d/box2d.h>

#include "Tileset.h"

namespace PlatformDataEngine {

	class Tileset;

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

