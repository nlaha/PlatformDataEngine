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

        SpriteRenderer();

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void networkSerialize(PDEPacket& output);
        void networkDeserialize(PDEPacket& input);

        void loadDefinition(nlohmann::json object);

        inline sf::Sprite& getSprite() { return this->m_sprite; };
        inline std::shared_ptr<sf::Texture> getTexture() const { return this->m_texture; };

        inline void setTexture(std::shared_ptr<sf::Texture> tex) 
        {
            this->m_texture = tex;
            this->m_sprite.setTexture(*this->m_texture, false);
        };

        inline void setRect(sf::IntRect rect) { this->m_rect = rect; };
        inline sf::IntRect getRect() const { return this->m_rect; };

    private:
        sf::Sprite m_sprite;
        std::shared_ptr<sf::Texture> m_texture;
        sf::IntRect m_rect;

    };
}

