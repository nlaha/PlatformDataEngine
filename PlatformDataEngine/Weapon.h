#pragma once
#include <memory>

#include "Component.h"
#include "PlayerInputManager.h"
#include "NetworkInputManager.h"
#include "Globals.h"

namespace PlatformDataEngine {

    /// <summary>
    /// The weapon generic component
    /// </summary>
    class Weapon :
        public Component
    {
    public:
        virtual void init();

        virtual void update(const float& dt, const float& elapsedTime);

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        virtual void copy(std::shared_ptr<Component> otherCompPtr);

        virtual void loadDefinition(nlohmann::json object);

        virtual void networkSerialize(PDEPacket& output);
        virtual void networkDeserialize(PDEPacket& input);

        inline bool isCoolingDown() const {
            return m_isCoolingDown;
        }

    protected:
        std::shared_ptr<InputManager> m_pInputManager;

        sf::Clock m_weaponClock;
        float m_Cooldown;
        float m_velocity;
        bool m_isCoolingDown;

        sf::Vector2i m_oldPixelPos;
        sf::Vector2f m_worldPos;
    };
}