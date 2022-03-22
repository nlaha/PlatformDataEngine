#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

#include "Component.h"

namespace PlatformDataEngine {

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

    private:
        b2Body* m_body;
        b2BodyType m_bodyType;
        std::shared_ptr<b2PolygonShape> m_bodyShape;
        std::shared_ptr<b2FixtureDef> m_bodyFixtureDef;

        sf::FloatRect m_bounds;

    };
}

