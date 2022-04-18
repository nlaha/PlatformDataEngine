#include "ParticleSystem.h"
#include "Utility.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

void ParticleSystem::init()
{
	Component::init();

	spawnParticles();
}

void ParticleSystem::update(const float &dt, const float &elapsedTime)
{
}

void ParticleSystem::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
}

void ParticleSystem::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<ParticleSystem> other = std::dynamic_pointer_cast<ParticleSystem>(otherCompPtr);

	*this = *other;
}

void ParticleSystem::loadDefinition(nlohmann::json object)
{
	this->m_numParticles = object.at("numParticles");
	this->m_velocity = object.at("velocity");
	this->m_particleName = object.at("particle");
	if (object.count("sound") > 0)
	{
		this->m_sound = PlatformDataEngineWrapper::getAudioSystem()->getSound(object.at("sound"));
	}
}

void ParticleSystem::spawnParticles()
{

	if (this->m_sound != nullptr)
	{
		// get random float between 0.5f and 1.5f for pitch
		float pitch = Utility::getRandomFloat(0.5f, 1.5f);
		this->m_sound->setPitch(pitch);
		this->m_sound->setAttenuation(0.02f);
		this->m_sound->setPosition(sf::Vector3f(this->m_parent->getPosition().x, this->m_parent->getPosition().y, 0.0f));
		this->m_sound->play();
	}

	for (size_t i = 0; i < this->m_numParticles; i++)
	{

		std::shared_ptr<GameObject> particle = PlatformDataEngineWrapper::getWorld()->spawnGameObject(
			this->m_particleName, this->m_parent->getPosition(), "", true);

		b2Body *bd = particle->findComponentOfType<PhysicsBody>()->getBody();

		b2Fixture *fix = bd->GetFixtureList();
		b2Filter filter;
		filter.categoryBits = PlatformDataEngine::PARTICLE;
		filter.maskBits = PlatformDataEngine::WORLD_STATIC | PlatformDataEngine::WORLD_DYNAMIC;
		fix->SetFilterData(filter);

		// get a random vector
		b2Vec2 dir = Utility::normalize(b2Vec2(
			rand() % 100 + (-100),
			rand() % 100 + (-100)));
		dir.x *= this->m_velocity / Constants::PHYS_SCALE;
		dir.y *= this->m_velocity / Constants::PHYS_SCALE;

		bd->ApplyLinearImpulseToCenter(dir, true);
	}
}
