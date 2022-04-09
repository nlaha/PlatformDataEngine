#pragma once
#include "Component.h"

namespace PlatformDataEngine {

    class DamageHandler :
        public Component
    {
    public:
        virtual void init() = 0;

        virtual void update(const float& dt, const float& elapsedTime) = 0;

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

        virtual void copy(std::shared_ptr<Component> otherCompPtr) = 0;

        virtual void loadDefinition(nlohmann::json object) = 0;

        virtual void onDeath() = 0;

        virtual void onDamage(float currentHP) = 0;
    };

}
