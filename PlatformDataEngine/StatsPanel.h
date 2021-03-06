#pragma once

#include <spdlog/spdlog.h>
#include <memory>

#include "Component.h"
#include "Utility.h"

namespace PlatformDataEngine
{

    class AnimationController;
    class RocketLauncher;

    class StatsPanel : public Component
    {
    public:
        void init();

        void update(const float &dt, const float &elapsedTime);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

    private:
        AnimationController* m_animController;
        GameObject* m_player;
        RocketLauncher* m_playerRocketLauncher;

        sf::RectangleShape m_healthBar;
        sf::RectangleShape m_healthBarDelayed;

        float m_health;
        float m_targetHealth;
        float m_healthBarSpeed;

        sf::Vector2f m_healthBarSize;
    };
}
