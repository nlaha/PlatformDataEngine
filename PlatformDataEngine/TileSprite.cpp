#include "TileSprite.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
TileSprite::TileSprite(std::shared_ptr<Tileset> tileset, b2PolygonShape& collisionShape, int tileIdx, bool hasPhysics)
{
	this->m_tileIdx = tileIdx;
	this->m_tileset = tileset;
	this->m_hasPhysics = hasPhysics;
	this->m_bodyShape = std::make_shared<b2PolygonShape>(collisionShape);

	initTransform();

	registerSelf();
}


/// <summary>
/// Copy constructor
/// </summary>
TileSprite::TileSprite(const TileSprite& other)
{
	this->m_tileIdx = other.m_tileIdx;
	this->m_tileset = other.m_tileset;

	initTransform();

	registerSelf();
}


/// <summary>
/// Constructor
/// </summary>
TileSprite::TileSprite(std::shared_ptr<Tileset> tileset, b2PolygonShape& collisionShape, int tileIdx, bool hasPhysics, sf::Vector2f position)
{
	this->m_tileIdx = tileIdx;
	this->m_tileset = tileset;
	this->m_hasPhysics = hasPhysics;
	this->m_bodyShape = std::make_shared<b2PolygonShape>(collisionShape);

	initTransform();

	this->setPosition(position);

	registerSelf(position);
}

TileSprite::~TileSprite()
{
	this->m_tileset->deregisterSprite(this);
}


/// <summary>
/// Register the sprite.
/// </summary>
void TileSprite::registerSelf(sf::Vector2f pos)
{
	// init static physics body
	
	if (this->m_hasPhysics) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody;

		bodyDef.position.Set(pos.x / Constants::PHYS_SCALE, pos.y / Constants::PHYS_SCALE);

		this->m_body = PlatformDataEngineWrapper::getWorld()->getPhysWorld()->CreateBody(&bodyDef);

		this->m_body->CreateFixture(this->m_bodyShape.get(), 0.0f);
	}

	this->m_tileset->registerSprite(this);
	
}


/// <summary>
/// Initialize the transform.
/// </summary>
void TileSprite::initTransform()
{
	// default (can be overriden)
}
