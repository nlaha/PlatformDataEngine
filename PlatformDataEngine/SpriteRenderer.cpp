#include "SpriteRenderer.h"
#include "GameObject.h"

void PlatformDataEngine::SpriteRenderer::init()
{
}

void PlatformDataEngine::SpriteRenderer::update(const float& dt, const float& elapsedTime)
{
}

void PlatformDataEngine::SpriteRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(this->m_sprite, states);
}

void PlatformDataEngine::SpriteRenderer::loadDefinition(nlohmann::json object)
{
    // loop through each key in the json object
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        // get the key and value
        std::string key = it.key();
        nlohmann::json value = it.value();
        
        this->m_properties.emplace(key, value);
    }

    nlohmann::json rectObj = this->m_properties.at("rect");
    this->m_rect = sf::IntRect(
        rectObj.at("x"),
        rectObj.at("y"),
        rectObj.at("width"),
        rectObj.at("height")
    );

    this->m_texture.loadFromFile(this->m_properties.at("texture"), this->m_rect);
    this->m_sprite.setTexture(this->m_texture);
    this->m_sprite.setColor(sf::Color(255, 255, 255));
}
