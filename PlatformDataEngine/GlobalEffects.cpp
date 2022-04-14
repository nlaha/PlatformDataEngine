#include "GlobalEffects.h"

#include <box2d/box2d.h>

#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

void GlobalEffects::explode(SpriteRenderer& sprite, sf::Vector2f pos, int slices)
{
	sf::IntRect fullRect = sprite.getRect();
	std::vector<sf::IntRect> particleRects;

	// get a bunch of sub-rects int the current sprite rect
	int width = std::abs(fullRect.width) / slices;
	int height = std::abs(fullRect.height) / slices;
	for (int i = 0; i < slices; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			particleRects.push_back(sf::IntRect(i * width + fullRect.left, j * height + fullRect.top, width, height));
		}
	}

	std::shared_ptr<sf::Texture> tex = sprite.getTexture();
	for (sf::IntRect pRect : particleRects)
	{
		std::shared_ptr<GameObject> particle = PlatformDataEngineWrapper::getWorld()->spawnGameObject(
			"ExplodeChunkParticle", pos + sf::Vector2f(pRect.left - fullRect.left, pRect.top - fullRect.top),
			"", true);
		particle->setZlayer(40);

		SpriteRenderer* pSprite = particle->findComponentOfType<SpriteRenderer>().get();
		pSprite->setTexture(tex);
		pSprite->setRect(pRect);
		pSprite->getSprite().setPosition(-(pRect.width / 2.0f), -(pRect.height / 2.0f));

		std::shared_ptr<PhysicsBody> pBody = particle->findComponentOfType<PhysicsBody>();

		// add fixture
		b2FixtureDef fd;
		b2PolygonShape shape;
		shape.SetAsBox(
			(pRect.width / 2.0f) / Constants::PHYS_SCALE, 
			(pRect.height / 2.0f) / Constants::PHYS_SCALE);

		fd.shape = &shape;
		fd.density = pBody->getDensity();
		fd.friction = pBody->getFriction();
		fd.restitution = pBody->getBouncy();
		fd.filter.categoryBits = PlatformDataEngine::PARTICLE;
		fd.filter.maskBits = PlatformDataEngine::WORLD_STATIC | PlatformDataEngine::WORLD_DYNAMIC;
		pBody->getBody()->CreateFixture(&fd);

	}
}
