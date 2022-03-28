#pragma once
#include <SFML/Graphics.hpp>
#include <execution>

#include "Component.h"
#include "RaycastCallback.h"
#include "PlayerInputManager.h"

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
    class AnimationController;

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
        bool fastGroundCheck() const;

        static bool HasFlag(int a, int b)
        {
            return (a & b) == b;
        }

    private:

        std::shared_ptr<PlayerInputManager> m_pInputManager;
        
        void updateAnimation(b2Vec2 velocity);
        
        std::shared_ptr<PhysicsBody> m_PhysBody;
        std::shared_ptr<AnimationController> m_AnimController;

        float m_moveForce;
        float m_jumpForce;
        float m_maxVelocity;

        sf::Clock m_jumpCooldownClock;
        sf::Clock m_dashCooldownClock;
        float m_jumpCooldown;
        float m_dashCooldown;

        bool m_prev_jump_state;
        bool m_prev_dash_state;
    };
}

