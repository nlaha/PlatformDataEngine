#include "TimedDestroy.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

void TimedDestroy::init()
{
	Component::init();

	this->m_timer = sf::Clock();
	if (this->m_useRange) {
		this->m_time = this->m_timeLow + (rand() % (int)this->m_timeHigh);
	}
}

void TimedDestroy::update(const float& dt, const float& elapsedTime)
{
	if (m_timer.getElapsedTime().asMilliseconds() > this->m_time) {
		this->m_parent->destroySelf();
	}
}

void TimedDestroy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void TimedDestroy::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<TimedDestroy> other = std::dynamic_pointer_cast<TimedDestroy>(otherCompPtr);

	*this = *other;
}

void TimedDestroy::loadDefinition(nlohmann::json object)
{
	this->m_time = object.at("time");
	this->m_useRange = object.at("range");
	this->m_timeLow = object.at("rangeLow");
	this->m_timeHigh = object.at("rangeHigh");
}
