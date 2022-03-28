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

	/// <summary>
	/// Every component that requires input will contain one of these.
	/// A player character controller for example might contain a player input manager
	/// </summary>
	class PlayerInputManager : public InputManager
	{
	public:

		PlayerInputManager(int gamepadIndex);

		class Axis : public InputManager::Axis {
		public:

			Axis(int gamepadIndex, float deadZone);
			float getValue();
			bool isPositive();
			bool isNegative();
			void addTrigger(sf::Joystick::Axis axis);
			void addTrigger(sf::Keyboard::Key key, bool direction);

		private:
			std::vector<sf::Joystick::Axis> m_joyAxis;
			std::vector < sf::Keyboard::Key> m_positiveKeys;
			std::vector < sf::Keyboard::Key> m_negativeKeys;
			int m_gamepadIndex;
			float m_deadZone;
		};

		class Button : public InputManager::Button {
		public:

			Button(int gamepadIndex);
			bool getValue();
			void addTrigger(int button);
			void addTrigger(sf::Keyboard::Key key);

		private:
			std::vector <int> m_buttons;
			std::vector <sf::Keyboard::Key> m_keys;
			int m_gamepadIndex;
		};

		PlayerInputManager::Axis& getAxis(std::string axisName);
		PlayerInputManager::Button& getButton(std::string button);

		virtual void loadDefinition(std::string inputManagerFile);

	private:
		std::map<std::string, Axis> m_axis;
		std::map<std::string, Button> m_buttons;
		int m_gamepadIndex;

	};
}

