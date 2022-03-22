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

		void update(const float& dt, const float& elapsedTime);

	private:
		std::shared_ptr<sf::View> m_view;
		std::shared_ptr<GameObject> m_gameObject;
		float m_lerpTime;

	};
}

