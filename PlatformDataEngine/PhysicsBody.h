#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

#include "Component.h"
#include "PhysicsCallbacks.h"

namespace PlatformDataEngine
{

    /// <summary>
    /// A component that enables collision and rigidbody
    /// for a gameObject
    /// </summary>
    class PhysicsBody : public Component
    {
    public:
        // constructor
        PhysicsBody();

        void copy(std::shared_ptr<Component> otherCompPtr);

        ~PhysicsBody();

        void init();

        void update(const float &dt, const float &elapsedTime);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);

        inline b2Body *getBody()
        {
            return this->m_body;
        }

        inline sf::FloatRect getBounds()
        {
            return this->m_bounds;
        }

        inline float getDensity() const { return this->m_density; };
        inline bool getIsContinuous() const { return this->m_isContinuous; };
        inline float getBouncy() const { return this->m_bouncy; };
        inline float getFriction() const { return this->m_friction; };
        inline bool getDoesRotate() const { return this->m_doesRotate; };

    private: 
        
        b2Body *m_body;
        b2BodyType m_bodyType;
        float m_density;
        bool m_isContinuous;
        float m_bouncy;
        float m_friction;
        bool m_doesRotate;
        PhysBodyUserData *m_bodyUserData;

        b2BodyDef m_bodyDef;
        std::vector<b2FixtureDef> m_bodyFixtureDefs;

        sf::FloatRect m_bounds;
    };
}
