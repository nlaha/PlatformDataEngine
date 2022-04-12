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
	/// A basic input manager for keyboard, mouse and gamepad input
	/// from a player
	/// </summary>
	class PlayerInputManager : public InputManager
	{
	public:

		sf::Vector2i getMouse();

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
			void addTrigger(sf::Mouse::Button button);

		private:
			std::vector <int> m_buttons;
			std::vector <sf::Keyboard::Key> m_keys;
			std::vector<sf::Mouse::Button> m_mouseBtns;
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

