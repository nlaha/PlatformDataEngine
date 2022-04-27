#include "PlayerInputManager.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

/// <summary>
/// Axis represents a range based input (-100 to 100)
/// </summary>
/// <param name="gamepadIndex"></param>
PlayerInputManager::Axis::Axis(int gamepadIndex, float deadZone)
{
	this->m_gamepadIndex = gamepadIndex;
	this->m_deadZone = deadZone;
}

/// <summary>
/// Adds a joystick axis as a trigger
/// </summary>
/// <param name="axis">the axis</param>
void PlayerInputManager::Axis::addTrigger(sf::Joystick::Axis axis)
{
	this->m_joyAxis.push_back(axis);
}

/// <summary>
/// Adds a button trigger for the axis
/// </summary>
/// <param name="key">the key to add</param>
/// <param name="direction">true - positive, false - negative</param>
void PlayerInputManager::Axis::addTrigger(sf::Keyboard::Key key, bool direction)
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
float PlayerInputManager::Axis::getValue() const
{
	float value = 0.0f;

	if (PlatformDataEngineWrapper::getWindow()->hasFocus()) {

		// check if anything is happening on the joystick axis
		for (sf::Joystick::Axis jAxis : this->m_joyAxis)
		{
			float rawVal = sf::Joystick::getAxisPosition(this->m_gamepadIndex, jAxis);
			if (std::abs(rawVal) > this->m_deadZone) {
				value += sf::Joystick::getAxisPosition(this->m_gamepadIndex, jAxis);
			}
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
	}

	return std::clamp(value, -100.0f, 100.0f);
}

/// <summary>
/// Gets if the value is positive
/// </summary>
/// <returns>true if positive, false if negative</returns>
bool PlayerInputManager::Axis::isPositive() const
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
bool PlayerInputManager::Axis::isNegative() const
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
PlayerInputManager::Button::Button(int gamepadIndex)
{
	this->m_gamepadIndex = gamepadIndex;
}

/// <summary>
/// Adds a button gamepad trigger
/// </summary>
/// <param name="button">the button</param>
void PlayerInputManager::Button::addTrigger(int button)
{
	this->m_buttons.push_back(button);
}

/// <summary>
/// Adds a button trigger key
/// </summary>
/// <param name="key">the key</param>
void PlayerInputManager::Button::addTrigger(sf::Keyboard::Key key)
{
	this->m_keys.push_back(key);
}

/// <summary>
/// Adds a mouse button trigger
/// </summary>
/// <param name="button">the mouse button</param>
void PlayerInputManager::Button::addTrigger(sf::Mouse::Button button)
{
	this->m_mouseBtns.push_back(button);
}

/// <summary>
/// Gets the value of the button
/// </summary>
/// <returns>true if pressed, false if not</returns>
bool PlayerInputManager::Button::getValue() const
{
	bool value = false;

	if (PlatformDataEngineWrapper::getWindow()->hasFocus()) {

		// check mouse buttons
		for (sf::Mouse::Button key : this->m_mouseBtns)
		{
			if (sf::Mouse::isButtonPressed(key))
			{
				value += true;
			}
		}

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
	}

	return value;
}

sf::Vector2f PlatformDataEngine::PlayerInputManager::getMouse()
{
	if (PlatformDataEngineWrapper::getWindow()->hasFocus()) {
		sf::Vector2i pixelPos = sf::Mouse::getPosition(*PlatformDataEngineWrapper::getWindow());
		return PlatformDataEngineWrapper::getWindow()->mapPixelToCoords(pixelPos);
	}
	else {
		return sf::Vector2f(0, 0);
	}
}

PlayerInputManager::PlayerInputManager(int gamepadIndex)
{
	this->m_gamepadIndex = gamepadIndex;
}

/// <summary>
/// Gets an axis by name
/// </summary>
/// <param name="axisName">the axis name</param>
/// <returns>the axis</returns>
PlayerInputManager::Axis& PlayerInputManager::getAxis(const std::string& axisName)
{
	return *this->m_axis.at(axisName);
}

/// <summary>
/// Gets a button by name
/// </summary>
/// <param name="button">the button name</param>
/// <returns></returns>
PlayerInputManager::Button& PlayerInputManager::getButton(const std::string& button)
{
	return *this->m_buttons.at(button);
}

/// <summary>
/// Loads input manager configuration from a json file
/// </summary>
/// <param name="inputManagerFile"></param>
void PlayerInputManager::loadDefinition(const std::string& inputManagerFile)
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
	sf::Uint8 axisCount = 0;
	for (const auto& axisConfig : inputConfig.at("axis").items())
	{
		std::shared_ptr<Axis> axis = std::make_shared<Axis>(this->m_gamepadIndex, axisConfig.value().at("deadZone"));

		// axis keys
		if (axisConfig.value().count("positiveKeys") > 0) {
			for (const auto& axisKey : axisConfig.value().at("positiveKeys"))
			{
				axis->addTrigger(static_cast<sf::Keyboard::Key>(axisKey), 1);
			}
		}

		if (axisConfig.value().count("negativeKeys") > 0) {
			for (const auto& axisKey : axisConfig.value().at("negativeKeys"))
			{
				axis->addTrigger(static_cast<sf::Keyboard::Key>(axisKey), 0);
			}
		}

		// axis gamepad axis
		for (const auto& axisAxis : axisConfig.value().at("axis"))
		{
			axis->addTrigger(static_cast<sf::Joystick::Axis>(axisAxis));
		}

		this->m_axisIdx.push_back(axis);
		this->m_axis.emplace(axisConfig.key(), axis);
		axisCount++;
	}

	// load buttons
	sf::Uint8 buttonCount = 0;
	for (const auto& buttonConfig : inputConfig.at("buttons").items())
	{
		std::shared_ptr<Button> button = std::make_shared<Button>(this->m_gamepadIndex);

		// button keys
		if (buttonConfig.value().count("keys") > 0)
		{
			for (const int& buttonKey : buttonConfig.value().at("keys"))
			{
				button->addTrigger(static_cast<sf::Keyboard::Key>(buttonKey));
			}
		}

		// button buttons
		if (buttonConfig.value().count("buttons") > 0)
		{
			for (const int& buttonButton : buttonConfig.value().at("buttons"))
			{
				button->addTrigger(buttonButton);
			}
		}

		if (buttonConfig.value().count("mouseButtons") > 0)
		{
			// button buttons
			for (const int& mouseButtonButton : buttonConfig.value().at("mouseButtons"))
			{
				button->addTrigger(static_cast<sf::Mouse::Button>(mouseButtonButton));
			}
		}

		this->m_buttonIdx.push_back(button);
		this->m_buttons.emplace(buttonConfig.key(), button);
		buttonCount++;
	}
}

void PlayerInputManager::serializeInputs(PDEPacket& packet)
{
	// grab axis values
	packet.setFlag(PDEPacket::UserInput);

	sf::Uint8 counter = 0; // todo, make a vector of inputs as well as map for sending indices
	for (const auto& axis : this->m_axisIdx)
	{
		if (axis->needsUpdate()) {
			packet << true;
			axis->setLast(axis->getValue());
			packet << counter << sf::Int8(axis->getValue());
		}
		counter++;
	}
	packet << false;

	// grab button values
	counter = 0;
	for (const auto& button : this->m_buttonIdx)
	{
		if (button->needsUpdate()) {
			packet << true;
			button->setLast(button->getValue());
			packet << counter << button->getValue();
		}
		counter++;
	}
	packet << false;

	sf::Vector2f mouse = this->getMouse();
	packet << mouse.x << mouse.y;
}
