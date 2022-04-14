#pragma once
#include <SFML/Graphics.hpp>
#include <execution>
#include <map>
#include <memory>

#include "Component.h"
#include "PhysicsCallbacks.h"
#include "PlayerInputManager.h"
#include "NetworkInputManager.h"

namespace PlatformDataEngine {

    enum DirTestMask {
        NONE,
        LEFT,
        RIGHT,
        UP,
        DOWN,
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

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);
        
        DirTestMask isAdjacentWall() const;
        bool fastGroundCheck() const;

        inline InputManager* getInputManager() const { return this->m_pInputManager.get(); };

        static bool HasFlag(int a, int b)
        {
            return (a & b) == b;
        }

    private:

        std::shared_ptr<InputManager> m_pInputManager;
        
        void updateAnimation(b2Vec2 velocity);
        
        PhysicsBody* m_PhysBody;
        AnimationController* m_AnimController;

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

