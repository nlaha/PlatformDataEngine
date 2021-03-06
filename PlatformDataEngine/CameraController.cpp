#include "CameraController.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
/// <param name="lerpTime">higher values result in faster camera reaction time</param>
/// <param name="view">a pointer to the world's view</param>
CameraController::CameraController(float lerpTime, std::shared_ptr<sf::View> view)
{
	this->m_lerpTime = lerpTime;
	this->m_gameObject = nullptr;
	this->m_view = view;
}

/// <summary>
/// Sets the gameObject for which the camera controller should follow
/// </summary>
/// <param name="gameObject">the game object to target</param>
void CameraController::setTarget(GameObject* gameObject)
{
	this->m_gameObject = gameObject;
}

void CameraController::update(const float& dt, const float& elapsedTime)
{
	// smoothly move to the target over time
	if (this->m_gameObject != nullptr)
	{
		sf::Vector2f targetPosition = this->m_gameObject->getPosition();
		sf::Vector2f currentPosition = this->m_view->getCenter();

		sf::Vector2f newPosition = Utility::lerp(currentPosition, targetPosition, this->m_lerpTime * dt);

		sf::Listener::setPosition(
			newPosition.x,
			newPosition.y,
			0.0f
		);

		this->m_view->setCenter(newPosition);
	}
}

sf::Vector2f CameraController::getUIOffset() const
{
	if (this->m_view != nullptr) {
		return this->m_view->getCenter() - (this->m_view->getSize() / 2.0f);
	}
	else {
		return sf::Vector2f(0.0f, 0.0f);
	}
}
