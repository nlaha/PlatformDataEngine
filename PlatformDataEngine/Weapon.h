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
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void networkSerialize(PDEPacket& output);
        void networkDeserialize(PDEPacket& input);

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