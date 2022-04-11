#include "GlobalEffects.h"

#include <box2d/box2d.h>

#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

void GlobalEffects::explode(SpriteRenderer sprite, sf::Vector2f pos)
{
	sf::IntRect fullRect = sprite.getRect();
	std::vector<sf::IntRect> particleRects;

	// get a bunch of sub-rects int the current sprite rect
	int width = std::abs(fullRect.width) / 8;
	int height = std::abs(fullRect.height) / 8;
	for (int i = 0; i < std::abs(fullRect.width); i += width)
	{
		for (int j = 0; j < std::abs(fullRect.height); j += height)
		{
			particleRects.push_back(sf::IntRect(i, j, width, height));
		}
	}

	std::shared_ptr<sf::Texture> tex = sprite.getTexture();
	for (sf::IntRect pRect : particleRects)
	{
		std::shared_ptr<GameObject> particle = PlatformDataEngineWrapper::getWorld()->spawnGameObject("ExplodeChunkParticle", pos);
		particle->setZlayer(40);

		std::shared_ptr<SpriteRenderer> pSprite = particle->findComponentOfType<SpriteRenderer>();
		pSprite->setTexture(tex);
		pSprite->setRect(pRect);

		std::shared_ptr<PhysicsBody> pBody = particle->findComponentOfType<PhysicsBody>();

		// add fixture
		b2FixtureDef fd;
		b2PolygonShape shape;
		shape.SetAsBox(pRect.width / 2.0f, pRect.height / 2.0f);

		fd.shape = &shape;
		fd.density = pBody->getDensity();
		fd.friction = pBody->getFriction();
		fd.restitution = pBody->getBouncy();
		fd.filter.categoryBits = PlatformDataEngine::PARTICLE;
		fd.filter.maskBits = PlatformDataEngine::WORLD_STATIC | PlatformDataEngine::WORLD_DYNAMIC;
		pBody->getBody()->CreateFixture(&fd);

		// get a random vector
		b2Vec2 dir = Utility::normalize(b2Vec2(
			rand() % 100 + (-100),
			rand() % 100 + (-100)));
		dir.x *= 10;
		dir.y *= 10;

		pBody->getBody()->ApplyLinearImpulseToCenter(dir, true);
	}
}
