#pragma once
#include <SFML/Graphics.hpp>
#include <execution>

#include "Component.h"
#include "RaycastCallback.h"

namespace PlatformDataEngine {

    enum GroundTestMask {
        NONE,
        LEFT,
        RIGHT,
        DOWN,
        DOWNRIGHT,
        DOWNLEFT,
    };

    class PhysicsBody;

    /// <summary>
    /// A component that handles player input and animations (if enabled)
    /// Precondition: if animations are enabled, the gameObject must have an 
    /// animation controller component
    /// </summary>
    class CharacterController :
        public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);
        
        int isAdjacentGround() const;

        static bool HasFlag(int a, int b)
        {
            return (a & b) == b;
        }

    private:
        std::shared_ptr<PhysicsBody> m_pBody;

        float m_moveForce;
        float m_jumpForce;
        float m_maxVelocity;

        bool m_prev_key_state;
    };
}

