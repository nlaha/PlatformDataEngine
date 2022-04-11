#pragma once
#include "DamageHandler.h"

namespace PlatformDataEngine {
    class PropDamageHandler :
        public DamageHandler
    {
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void onDeath();

        void onDamage(float currentHP);
    };
}

