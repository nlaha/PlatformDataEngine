#pragma once
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "InputManager.h"

namespace PlatformDataEngine {

	class NetworkInputManager : public InputManager
	{
	public:

		inline sf::Vector2i getMouse() { return sf::Vector2i(0.0, 0.0); };

		class Axis : public InputManager::Axis {
		public:
			float getValue();
			bool isPositive();
			bool isNegative();

		private:
			float value;
		};

		class Button : public InputManager::Button {
		public:
			bool getValue();

		private:
			bool value;
		};

		NetworkInputManager::Axis& getAxis(std::string axisName);
		NetworkInputManager::Button& getButton(std::string button);

		void loadDefinition(std::string inputManagerFile);

	private:
		std::map<std::string, Axis> m_axis;
		std::map<std::string, Button> m_buttons;
	};

}
