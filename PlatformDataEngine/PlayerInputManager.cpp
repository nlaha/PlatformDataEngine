#include "PlayerInputManager.h"

using namespace PlatformDataEngine;

/// <summary>
/// Axis represents a range based input (-100 to 100)
/// </summary>
/// <param name="gamepadIndex"></param>
PlatformDataEngine::PlayerInputManager::Axis::Axis(int gamepadIndex, float deadZone)
{
	this->m_gamepadIndex = gamepadIndex;
	this->m_deadZone = deadZone;
}

/// <summary>
/// Adds a joystick axis as a trigger
/// </summary>
/// <param name="axis">the axis</param>
void PlatformDataEngine::PlayerInputManager::Axis::addTrigger(sf::Joystick::Axis axis)
{
	this->m_joyAxis.push_back(axis);
}

/// <summary>
/// Adds a button trigger for the axis
/// </summary>
/// <param name="key">the key to add</param>
/// <param name="direction">true - positive, false - negative</param>
void PlatformDataEngine::PlayerInputManager::Axis::addTrigger(sf::Keyboard::Key key, bool direction)
{
	if (direction)
	{
		this->m_positiveKeys.push_back(key);
	}
	else
	{
		this->m_negativeKeys.push_back(key);
	}
}

/// <summary>
/// Gets the value from the axis
/// </summary>
/// <returns>the value from -100 to 100</returns>
float PlatformDataEngine::PlayerInputManager::Axis::getValue()
{
	float value = 0.0f;

	// check if anything is happening on the joystick axis
	for (sf::Joystick::Axis jAxis : this->m_joyAxis)
	{
		value += sf::Joystick::getAxisPosition(this->m_gamepadIndex, jAxis);
	}

	// check positive keys
	for (sf::Keyboard::Key key : this->m_positiveKeys)
	{
		if (sf::Keyboard::isKeyPressed(key))
		{
			value += 100;
		}
	}

	// check negative keys
	for (sf::Keyboard::Key key : this->m_negativeKeys)
	{
		if (sf::Keyboard::isKeyPressed(key))
		{
			value += -100;
		}
	}

	return std::clamp(value, -100.0f, 100.0f);
}

/// <summary>
/// Gets if the value is positive
/// </summary>
/// <returns>true if positive, false if negative</returns>
bool PlatformDataEngine::PlayerInputManager::Axis::isPositive()
{
	if (getValue() > this->m_deadZone)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/// <summary>
/// Gets if the value is negative
/// </summary>
/// <returns>true if positive, false if negative</returns>
bool PlatformDataEngine::PlayerInputManager::Axis::isNegative()
{
	if (getValue() < -this->m_deadZone)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/// <summary>
/// Button represents a true/false input
/// </summary>
/// <param name="gamepadIndex"></param>
PlatformDataEngine::PlayerInputManager::Button::Button(int gamepadIndex)
{
	this->m_gamepadIndex = gamepadIndex;
}

/// <summary>
/// Adds a button gamepad trigger
/// </summary>
/// <param name="button">the button</param>
void PlatformDataEngine::PlayerInputManager::Button::addTrigger(int button)
{
	this->m_buttons.push_back(button);
}

/// <summary>
/// Adds a button trigger key
/// </summary>
/// <param name="key">the key</param>
void PlatformDataEngine::PlayerInputManager::Button::addTrigger(sf::Keyboard::Key key)
{
	this->m_keys.push_back(key);
}

/// <summary>
/// Gets the value of the button
/// </summary>
/// <returns>true if pressed, false if not</returns>
bool PlatformDataEngine::PlayerInputManager::Button::getValue()
{
	bool value = false;

	// check keys
	for (sf::Keyboard::Key key : this->m_keys)
	{
		if (sf::Keyboard::isKeyPressed(key))
		{
			value += true;
		}
	}

	// check gamepad buttons
	for (int key : this->m_buttons)
	{
		if (sf::Joystick::isButtonPressed(this->m_gamepadIndex, key))
		{
			value += true;
		}
	}

	return value;
}

PlatformDataEngine::PlayerInputManager::PlayerInputManager(int gamepadIndex)
{
	this->m_gamepadIndex = gamepadIndex;
}

/// <summary>
/// Gets an axis by name
/// </summary>
/// <param name="axisName">the axis name</param>
/// <returns>the axis</returns>
PlatformDataEngine::PlayerInputManager::Axis& PlatformDataEngine::PlayerInputManager::getAxis(std::string axisName)
{
	return this->m_axis.at(axisName);
}

/// <summary>
/// Gets a button by name
/// </summary>
/// <param name="button">the button name</param>
/// <returns></returns>
PlatformDataEngine::PlayerInputManager::Button& PlatformDataEngine::PlayerInputManager::getButton(std::string button)
{
	return this->m_buttons.at(button);
}

/// <summary>
/// Loads input manager configuration from a json file
/// </summary>
/// <param name="inputManagerFile"></param>
void PlatformDataEngine::PlayerInputManager::loadDefinition(std::string inputManagerFile)
{
	std::ifstream configFile(inputManagerFile);

	if (!configFile.is_open())
	{
		spdlog::error("Failed to open input config file: {}", inputManagerFile);
		return;
	}
	spdlog::info("Loading input config file: {}", inputManagerFile);

	nlohmann::json inputConfig;
	configFile >> inputConfig;

	this->m_gamepadIndex = inputConfig.at("gamepadIndex");

	// load axis first
	for (const auto& axisConfig : inputConfig.at("axis").items())
	{
		Axis axis(this->m_gamepadIndex, axisConfig.value().at("deadZone"));

		// axis keys
		for (const auto& axisKey : axisConfig.value().at("positiveKeys"))
		{
			axis.addTrigger(static_cast<sf::Keyboard::Key>(axisKey), 1);
		}

		for (const auto& axisKey : axisConfig.value().at("negativeKeys"))
		{
			axis.addTrigger(static_cast<sf::Keyboard::Key>(axisKey), 0);
		}

		// axis gamepad axis
		for (const auto& axisAxis : axisConfig.value().at("axis"))
		{
			axis.addTrigger(static_cast<sf::Joystick::Axis>(axisAxis));
		}

		this->m_axis.emplace(axisConfig.key(), axis);
	}

	// load buttons
	for (const auto& buttonConfig : inputConfig.at("buttons").items())
	{
		Button button(this->m_gamepadIndex);

		// button keys
		for (const int& buttonKey : buttonConfig.value().at("keys"))
		{
			button.addTrigger(static_cast<sf::Keyboard::Key>(buttonKey));
		}

		// button buttons
		for (const int& buttonButton : buttonConfig.value().at("buttons"))
		{
			button.addTrigger(buttonButton);
		}

		this->m_buttons.emplace(buttonConfig.key(), button);
	}
}
