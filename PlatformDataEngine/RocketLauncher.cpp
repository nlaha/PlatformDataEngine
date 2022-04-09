#include "RocketLauncher.h"
#include "GameObject.h"
#include "Utility.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

void RocketLauncher::init()
{
	Component::init();

	this->m_pInputManager = PlatformDataEngineWrapper::getPlayerInputManager();
}

void RocketLauncher::update(const float& dt, const float& elapsedTime)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(*PlatformDataEngineWrapper::getWindow());
	sf::Vector2f worldPos = PlatformDataEngineWrapper::getWindow()->mapPixelToCoords(pixelPos);

	std::shared_ptr<GameObject> parent = this->m_parent->getParent();
	

	float rot = Utility::lookAt(
		parent->getPosition(),
		worldPos
	);

	this->m_parent->setRotation(rot);


	if (this->m_pInputManager->getButton("primary").getValue() && 
		this->m_rocketClock.getElapsedTime().asMilliseconds() >
		this->m_rocketCooldown) {
		sf::Vector2f directionVec = Utility::directionVec(parent->getPosition(), worldPos);
		std::shared_ptr<GameObject> p_gameObject = PlatformDataEngineWrapper::getWorld()->spawnGameObject(
			"RocketProjectile", 
			(parent->getPosition() + this->m_parent->getPosition())
		);
		p_gameObject->setZlayer(40);
		p_gameObject->setRotation(rot);
	
		b2Body* pBody = p_gameObject->findComponentOfType<PhysicsBody>()->getBody();
		pBody->SetTransform(pBody->GetPosition(), Utility::degToRad(rot));

		pBody->ApplyLinearImpulseToCenter(
			Utility::fromSf(directionVec * -this->m_velocity), true);

		std::shared_ptr<AnimationController> animControl = this->m_parent->findComponentOfType<AnimationController>();
		animControl->setAnimation("Shoot", 1.0, false);

		std::shared_ptr<RocketProjectile> projectile = p_gameObject->findComponentOfType<RocketProjectile>();
		projectile->setOwner(this->m_parent->getParent());

		this->m_rocketClock.restart();
	}
}

void RocketLauncher::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void RocketLauncher::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<RocketLauncher> other = std::dynamic_pointer_cast<RocketLauncher>(otherCompPtr);

	*this = *other;
}

void RocketLauncher::loadDefinition(nlohmann::json object)
{
	this->m_rocketCooldown = object.at("cooldown");
	this->m_velocity = object.at("velocity");
}
