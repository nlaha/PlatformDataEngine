#pragma once
#include <SFML/Graphics.hpp>

#include "Component.h"

namespace PlatformDataEngine {

    /// <summary>
    /// A component that simply holds an SFML sprite and renders it.
    /// </summary>
    class SpriteRenderer :
        public Component
    {
    public:

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void loadDefinition(nlohmann::json object);

        inline std::shared_ptr<sf::Sprite> getSprite() const { return this->m_sprite; };

        inline void setRect(sf::IntRect rect) { this->m_rect = rect; };

    private:
        std::shared_ptr<sf::Sprite> m_sprite;
        sf::Texture m_texture;
        sf::IntRect m_rect;

    };
}

