#pragma once
#include "Component.h"
namespace PlatformDataEngine {

    /// <summary>
    /// A component that manages playing animations on a gameObject.
    /// Precondition: the game object must have a sprite renderer component.
    /// </summary>
    class AnimationController : public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);
    };
}
