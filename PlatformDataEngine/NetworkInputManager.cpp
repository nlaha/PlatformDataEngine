#include "NetworkInputManager.h"

using namespace PlatformDataEngine;

bool NetworkInputManager::Button::getValue()
{
	return this->value;
}

float NetworkInputManager::Axis::getValue()
{
	return this->value;
}

// deadzones are going to be handled on the client side
// data won't be sent if it's under the deadzone
bool NetworkInputManager::Axis::isNegative()
{
	return this->value < 0.0f;
}

bool NetworkInputManager::Axis::isPositive()
{
	return this->value > 0.0f;
}

NetworkInputManager::Axis& NetworkInputManager::getAxis(std::string axisName)
{
	return this->m_axis.at(axisName);
}

NetworkInputManager::Button& NetworkInputManager::getButton(std::string button)
{
	return this->m_buttons.at(button);
}

void NetworkInputManager::loadDefinition(std::string inputManagerFile)
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

	// load axis first
	for (const auto& axisConfig : inputConfig.at("axis").items())
	{
		Axis axis;

		this->m_axis.emplace(axisConfig.key(), axis);
	}

	// load buttons
	for (const auto& buttonConfig : inputConfig.at("buttons").items())
	{
		Button button;

		this->m_buttons.emplace(buttonConfig.key(), button);
	}
}
