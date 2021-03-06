#include "StatsPanel.h"
#include "AnimationController.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"
#include "RocketLauncher.h"

using namespace PlatformDataEngine;

void StatsPanel::init()
{
    AnimationController *ac = this->m_parent->findComponentOfType<AnimationController>().get();
    if (ac != nullptr)
    {
        this->m_animController = ac;
    }
    else
    {
        spdlog::critical("GameObject {} has a StatsPanel so it must also have an AnimationController", this->m_parent->getId());
    }

    this->m_healthBar.setSize(sf::Vector2f(this->m_healthBarSize.x, this->m_healthBarSize.y));
    this->m_healthBar.setFillColor(sf::Color::Red);

    this->m_healthBarDelayed = this->m_healthBar;
    this->m_healthBarDelayed.setFillColor(sf::Color::White);
}

void StatsPanel::update(const float &dt, const float &elapsedTime)
{
    if (this->m_player == nullptr || this->m_playerRocketLauncher == nullptr)
    {
        this->m_player = PlatformDataEngineWrapper::getWorld()->getGameObject(
            PlatformDataEngineWrapper::getNetworkHandler()->getConnection()->id).get();

        //if (PlatformDataEngineWrapper::getWorld()->getPlayer() != nullptr)
        //{
        //    this->m_player = PlatformDataEngineWrapper::getWorld()->getPlayer();
        //    if (this->m_player->getChildren().size() > 0)
        //    {
        //        this->m_playerRocketLauncher = this->m_player->getChildren()[0]->findComponentOfType<RocketLauncher>().get();
        //    }
        //}
    }

    if (this->m_playerRocketLauncher != nullptr)
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
    }

    if (this->m_player != nullptr)
    {
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
}

void StatsPanel::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(this->m_healthBarDelayed, states);
    target.draw(this->m_healthBar, states);
}

void StatsPanel::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<StatsPanel> other = std::dynamic_pointer_cast<StatsPanel>(otherCompPtr);

    *this = *other;
}

void StatsPanel::loadDefinition(nlohmann::json object)
{
    this->m_healthBarSpeed = object.at("healthBar").at("healthBarSpeed");
    this->m_healthBar.setPosition({object.at("healthBar").at("transform").at("x"),
                                   object.at("healthBar").at("transform").at("y")});

    this->m_healthBarSize = sf::Vector2f(
        object.at("healthBar").at("transform").at("width"),
        object.at("healthBar").at("transform").at("height"));
}
