#include "NetworkInputManager.h"

using namespace PlatformDataEngine;

bool NetworkInputManager::Button::getValue() const
{
	return this->value;
}

float NetworkInputManager::Axis::getValue() const
{
	return this->value;
}

// deadzones are going to be handled on the client side
// data won't be sent if it's under the deadzone
bool NetworkInputManager::Axis::isNegative() const
{
	return this->value < 0.0f;
}

bool NetworkInputManager::Axis::isPositive() const
{
	return this->value > 0.0f;
}

NetworkInputManager::Axis& NetworkInputManager::getAxis(const std::string& axisName)
{
	return *this->m_axis.at(axisName);
}

NetworkInputManager::Button& NetworkInputManager::getButton(const std::string& button)
{
	return *this->m_buttons.at(button);
}

void NetworkInputManager::loadDefinition(const std::string& inputManagerFile)
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
		std::shared_ptr<Axis> axis = std::make_shared<Axis>();

		this->m_axisIdx.push_back(axis);
		this->m_axis.emplace(axisConfig.key(), axis);
	}

	// load buttons
	for (const auto& buttonConfig : inputConfig.at("buttons").items())
	{
		std::shared_ptr<Button> button = std::make_shared<Button>();

		this->m_buttonIdx.push_back(button);
		this->m_buttons.emplace(buttonConfig.key(), button);
	}
}

void NetworkInputManager::setAxis(const std::string& axisName, float value)
{
	if (this->m_axis.count(axisName) > 0) {
		this->m_axis.at(axisName)->setValue(value);
	}
}

void NetworkInputManager::setButton(const std::string& buttonName, bool value)
{
	if (this->m_buttons.count(buttonName) > 0) {
		this->m_buttons.at(buttonName)->setValue(value);
	}
}

void NetworkInputManager::setAxis(sf::Uint8 idx, float value)
{
	if (idx < this->m_axisIdx.size()) {
		this->m_axisIdx[idx]->setValue(value);
	}
}

void NetworkInputManager::setButton(sf::Uint8 idx, bool value)
{
	if (idx < this->m_buttonIdx.size()) {
		this->m_buttonIdx[idx]->setValue(value);
	}
}

void NetworkInputManager::setMouse(sf::Vector2f pos)
{
	this->m_mouse = pos;
}
