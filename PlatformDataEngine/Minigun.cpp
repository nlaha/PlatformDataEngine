#include <map>

#include "MiniGun.h"
#include "GameObject.h"
#include "Utility.h"
#include "Server.h"
#include "InputManager.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;



void MiniGun::update(const float& dt, const float& elapsedTime)
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
					"Bullet",
					(parent->getPosition() + this->m_parent->getPosition())
				).get();

				p_gameObject->setZlayer(40);
				p_gameObject->setRotation(rot);

				b2Body* pBody = p_gameObject->findComponentOfType<PhysicsBody>()->getBody();
				pBody->SetTransform(pBody->GetPosition(), Utility::degToRad(rot));

				pBody->ApplyLinearImpulseToCenter(
					Utility::fromSf(directionVec * -this->m_velocity), true);

				AnimationController* animControl = this->m_parent->findComponentOfType<AnimationController>().get();
				if (animControl->getAnimName() != "Shoot") {
					animControl->setAnimation("Shoot", 6.0, false);
				}

				Bullet* projectile = p_gameObject->findComponentOfType<Bullet>().get();
				projectile->setOwner(this->m_parent->getParent());

				this->m_weaponClock.restart();
			}
			else {
				this->m_isCoolingDown = true;
			}
		}
	}
}



void MiniGun::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<MiniGun> other = std::dynamic_pointer_cast<MiniGun>(otherCompPtr);

	*this = *other;
}