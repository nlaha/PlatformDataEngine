#pragma once
#include "Component.h"
#include "PlayerInputManager.h"
#include "Globals.h"

namespace PlatformDataEngine {

    /// <summary>
    /// The rocket launcher component
    /// TODO: make this into a more generic "weapon" class
    /// </summary>
    class RocketLauncher :
        public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        inline bool isCoolingDown() const {
            return m_rocketClock.getElapsedTime().asMilliseconds() < m_rocketCooldown;
        }

    private:
        std::shared_ptr<PlayerInputManager> m_pInputManager;

        sf::Clock m_rocketClock;
        float m_rocketCooldown;
        float m_velocity;

        sf::Vector2i m_oldPixelPos;
        sf::Vector2f m_worldPos;
    };
}

