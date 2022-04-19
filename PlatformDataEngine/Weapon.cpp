#include <map>

#include "RocketLauncher.h"
#include "GameObject.h"
#include "Utility.h"
#include "Server.h"
#include "InputManager.h"
#include "PlatformDataEngineWrapper.h"
#include "Weapon.h"

using namespace PlatformDataEngine;

void Weapon::init()
{
	Component::init();
	this->m_isCoolingDown = false;

	if (this->m_parent->getParent().get() == PlatformDataEngineWrapper::getWorld()->getPlayer()) {
		this->m_pInputManager = PlatformDataEngineWrapper::getPlayerInputManager();
	}
	else {
		this->m_pInputManager = std::make_shared<NetworkInputManager>();
		std::shared_ptr<NetworkInputManager> netIn = std::dynamic_pointer_cast<NetworkInputManager>(this->m_pInputManager);
		netIn->loadDefinition("./game/input.json");
		if (!PlatformDataEngineWrapper::getIsClient()) {
			dynamic_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler())->addInputManager(this->m_parent->getParent()->getConnection(), netIn);
		}
	}
}

void Weapon::update(const float& dt, const float& elapsedTime)
{
	if (!PlatformDataEngineWrapper::getIsClient()) {
		sf::Vector2f worldPos = this->m_pInputManager->getMouse();

		std::shared_ptr<GameObject> parent = this->m_parent->getParent();

		if (parent != nullptr) {
			if (this->m_pInputManager->getAxis("x_right").isNegative() || this->m_pInputManager->getAxis("x_right").isPositive()
				|| this->m_pInputManager->getAxis("y_right").isNegative() || this->m_pInputManager->getAxis("y_right").isNegative())
			{
				sf::Vector2f axisIn(this->m_pInputManager->getAxis("x_right").getValue(), this->m_pInputManager->getAxis("y_right").getValue());

				worldPos = parent->getPosition();
				worldPos.x += axisIn.x;
				worldPos.y += axisIn.y;
			}

			float rot = Utility::lookAt(
				parent->getPosition(),
				worldPos
			);

			this->m_parent->setRotation(rot);


			if (this->m_pInputManager->getButton("primary").getValue() &&
				this->m_weaponClock.getElapsedTime().asMilliseconds() >
				this->m_Cooldown) {
				this->m_isCoolingDown = false;
				sf::Vector2f directionVec = Utility::directionVec(parent->getPosition(), worldPos);
				GameObject* p_gameObject = PlatformDataEngineWrapper::getWorld()->spawnGameObject(
					"RocketProjectile",
					(parent->getPosition() + this->m_parent->getPosition())
				).get();
				p_gameObject->setZlayer(40);
				p_gameObject->setRotation(rot);

				b2Body* pBody = p_gameObject->findComponentOfType<PhysicsBody>()->getBody();
				pBody->SetTransform(pBody->GetPosition(), Utility::degToRad(rot));

				pBody->ApplyLinearImpulseToCenter(
					Utility::fromSf(directionVec * -this->m_velocity), true);

				AnimationController* animControl = this->m_parent->findComponentOfType<AnimationController>().get();
				animControl->setAnimation("Shoot", 1.0, false);

				RocketProjectile* projectile = p_gameObject->findComponentOfType<RocketProjectile>().get();
				projectile->setOwner(this->m_parent->getParent());

				this->m_weaponClock.restart();
			}
			else {
				this->m_isCoolingDown = true;
			}
		}
	}
}

void Weapon::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void Weapon::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<Weapon> other = std::dynamic_pointer_cast<Weapon>(otherCompPtr);

	*this = *other;
}

void Weapon::loadDefinition(nlohmann::json object)
{
	this->m_Cooldown = object.at("cooldown");
	this->m_velocity = object.at("velocity");
}

void Weapon::networkSerialize(PDEPacket& output)
{
	output << this->m_isCoolingDown;
}

void Weapon::networkDeserialize(PDEPacket& input)
{
	input >> this->m_isCoolingDown;
}