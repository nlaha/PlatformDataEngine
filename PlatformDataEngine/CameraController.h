#pragma once
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>

#include "GameObject.h"

namespace PlatformDataEngine {

	/// <summary>
	/// Handles moving the camera and targetting
	/// gameObjects
	/// </summary>
	class CameraController
	{
	public:
		CameraController(float lerpTime = 0.0f, std::shared_ptr<sf::View> view = nullptr);

		void setTarget(std::shared_ptr<GameObject> gameObject);
		inline std::shared_ptr<GameObject> getTarget() const { return this->m_gameObject; };

		void update(const float& dt, const float& elapsedTime);

		sf::Vector2f getUIOffset() const;

	private:
		std::shared_ptr<sf::View> m_view;
		std::shared_ptr<GameObject> m_gameObject;
		float m_lerpTime;

	};
}

