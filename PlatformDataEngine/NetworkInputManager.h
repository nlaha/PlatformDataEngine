#pragma once
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "InputManager.h"

namespace PlatformDataEngine {

	class NetworkInputManager : public InputManager
	{
	public:

		inline sf::Vector2f getMouse() { return this->m_mouse; };

		class Axis : public InputManager::Axis {
		public:
			float getValue() const;
			bool isPositive() const;
			bool isNegative() const;
			inline void setValue(float value) { this->value = value; };

		private:
			float value;
		};

		class Button : public InputManager::Button {
		public:
			bool getValue() const;
			inline void setValue(bool value) { this->value = value; };

		private:
			bool value;
		};

		NetworkInputManager::Axis& getAxis(std::string axisName);
		NetworkInputManager::Button& getButton(std::string button);

		void loadDefinition(std::string inputManagerFile);

		void setAxis(std::string axisName, float value);
		void setAxis(sf::Uint8 idx, float value);

		void setButton(std::string buttonName, bool value);
		void setButton(sf::Uint8 idx, bool value);

		void setMouse(sf::Vector2f pos);

	private:
		std::map<std::string, std::shared_ptr<Axis>> m_axis;
		std::map<std::string, std::shared_ptr<Button>> m_buttons;

		std::vector<std::shared_ptr<Axis>> m_axisIdx;
		std::vector<std::shared_ptr<Button>> m_buttonIdx;

		sf::Vector2f m_mouse;
	};

}
