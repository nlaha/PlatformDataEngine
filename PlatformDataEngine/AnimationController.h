#pragma once
#include "Component.h"
namespace PlatformDataEngine {

    class SpriteRenderer;

    /// <summary>
    /// A component that manages playing animations on a gameObject.
    /// Precondition: the game object must have a sprite renderer component.
    /// </summary>
    class AnimationController : public Component
    {
    public:
        
        enum FlipFlags {
            NONE,
            HORIZONTAL,
            VERTICAL
        };

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);

        inline void setFlipFlag(FlipFlags flip) { this->m_flip = flip;  };
        
    private:
        std::shared_ptr<SpriteRenderer> m_spriteRenderer;

        std::string m_currentAnim;
        bool m_loop;
        float m_speed;
        FlipFlags m_flip;
    };
}
