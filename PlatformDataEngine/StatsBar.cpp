#include "StatsBar.h"

using namespace PlatformDataEngine;

void StatsBar::init()
{
    this->m_healthBar.setSize(sf::Vector2f(this->m_healthBarSize.x, this->m_healthBarSize.y));
    this->m_healthBar.setFillColor(sf::Color::Red);

    this->m_healthBarDelayed = this->m_healthBar;
    this->m_healthBarDelayed.setFillColor(sf::Color::White);

    this->m_healthBarBackground = this->m_healthBar;
    this->m_healthBarBackground.setFillColor(sf::Color(80, 80, 80));
}

void StatsBar::update(const float& dt, const float& elapsedTime, float hp)
{
    this->m_targetHealth = hp;

    // lerp health to target health
    if (this->m_health != this->m_targetHealth)
    {
        this->m_health = Utility::lerp(this->m_health, this->m_targetHealth, this->m_healthBarSpeed * dt);
    }

    this->m_healthBar.setSize(sf::Vector2f(
        this->m_healthBarSize.x * (this->m_targetHealth / 100.0f), this->m_healthBarSize.y));
    this->m_healthBarDelayed.setSize(sf::Vector2f(
        this->m_healthBarSize.x * (this->m_health / 100.0f), this->m_healthBarSize.y));
}

void StatsBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::RenderStates statesNoRot = states;
    statesNoRot.transform = sf::Transform::Identity;
    statesNoRot.transform.translate(this->m_position + this->m_healthBar.getPosition());

    target.draw(this->m_healthBarBackground, statesNoRot);
    target.draw(this->m_healthBarDelayed, statesNoRot);
    target.draw(this->m_healthBar, statesNoRot);
}

void StatsBar::loadDefinition(nlohmann::json object)
{
    this->m_healthBarSpeed = object.at("healthBarSpeed");
    this->m_healthBar.setPosition({
        object.at("transform").at("x"),
        object.at("transform").at("y")
        });

    this->m_healthBarSize = sf::Vector2f(
        object.at("transform").at("width"),
        object.at("transform").at("height")
    );
}

/// <summary>
/// Sets the position of the stats bar
/// </summary>
/// <param name="pos">the new position</param>
void StatsBar::setPosition(sf::Vector2f pos)
{
    this->m_position = pos;
}

void StatsBar::networkSerialize(PDEPacket& output)
{
    output << this->m_position.x << this->m_position.y;
    output << this->m_targetHealth;
    output << this->m_health;
}

void StatsBar::networkDeserialize(PDEPacket& input)
{
    input >> this->m_position.x >> this->m_position.y;
    input >> this->m_targetHealth;
    input >> this->m_health;

    this->m_healthBar.setSize(sf::Vector2f(
        this->m_healthBarSize.x * (this->m_targetHealth / 100.0f), this->m_healthBarSize.y));
    this->m_healthBarDelayed.setSize(sf::Vector2f(
        this->m_healthBarSize.x * (this->m_health / 100.0f), this->m_healthBarSize.y));
}
