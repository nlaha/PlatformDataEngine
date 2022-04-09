#include "StatsPanel.h"
#include "AnimationController.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"
#include "RocketLauncher.h"

void PlatformDataEngine::StatsPanel::init()
{
    std::shared_ptr<AnimationController> ac = this->m_parent->findComponentOfType<AnimationController>();
    if (ac.get() != nullptr)
    {
        this->m_animController = ac;
    }
    else
    {
        spdlog::critical("GameObject {} has a StatsPanel so it must also have an AnimationController", this->m_parent->getName());
    }

    this->m_player = PlatformDataEngineWrapper::getWorld()->getPlayer();
    this->m_playerRocketLauncher = this->m_player->getChildren()[0]->findComponentOfType<RocketLauncher>();

    this->m_healthBar.setSize(sf::Vector2f(this->m_healthBarSize.x, this->m_healthBarSize.y));
    this->m_healthBar.setFillColor(sf::Color::Red);

    this->m_healthBarDelayed = this->m_healthBar;
    this->m_healthBarDelayed.setFillColor(sf::Color::White);
}

void PlatformDataEngine::StatsPanel::update(const float &dt, const float &elapsedTime)
{
    // update rocket cooldown animations
    if (m_playerRocketLauncher->isCoolingDown())
    {
        this->m_animController->setAnimation("IdleRocketCooldown");
    }
    else
    {
        this->m_animController->setAnimation("IdleRocketReady");
    }

    this->m_targetHealth = this->m_player->getHealth();

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

void PlatformDataEngine::StatsPanel::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(this->m_healthBarDelayed, states);
    target.draw(this->m_healthBar, states);
}

void PlatformDataEngine::StatsPanel::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<StatsPanel> other = std::dynamic_pointer_cast<StatsPanel>(otherCompPtr);

    *this = *other;
}

void PlatformDataEngine::StatsPanel::loadDefinition(nlohmann::json object)
{
    this->m_healthBarSpeed = object.at("healthBar").at("healthBarSpeed");
    this->m_healthBar.setPosition({ 
        object.at("healthBar").at("transform").at("x"), 
        object.at("healthBar").at("transform").at("y") 
    });

    this->m_healthBarSize = sf::Vector2f(
        object.at("healthBar").at("transform").at("width"),
        object.at("healthBar").at("transform").at("height")
    );
}
