#pragma once
#include "Component.h"

namespace PlatformDataEngine {

    /// <summary>
    /// Base damage handler class
    /// responsible for handling onDeath and onDamage events from the Alive class
    /// </summary>
    class DamageHandler :
        public Component
    {
    public:
        DamageHandler();

        virtual void init() = 0;

        virtual void update(const float& dt, const float& elapsedTime) = 0;

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

        virtual void copy(std::shared_ptr<Component> otherCompPtr) = 0;

        virtual void loadDefinition(nlohmann::json object) = 0;

        virtual void onDeath() = 0;

        virtual void onDamage(float currentHP) = 0;

    protected:
        bool m_died;
    };

}
