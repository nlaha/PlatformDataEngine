#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

#include "Component.h"

namespace PlatformDataEngine {

    /// <summary>
    /// A component that enables collision and rigidbody
    /// for a gameObject
    /// </summary>
    class PhysicsBody : public Component
    {
    public:

        ~PhysicsBody();

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);

        inline b2Body* getBody() {
            return this->m_body;
        }

        inline sf::FloatRect getBounds()
        {
            return this->m_bounds;
        }

    private:
        b2Body* m_body;
        b2BodyType m_bodyType;
        float m_density;
        float m_bouncy;
        float m_friction;

        sf::FloatRect m_bounds;

    };
}

