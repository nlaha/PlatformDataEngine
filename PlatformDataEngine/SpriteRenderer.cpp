#include "SpriteRenderer.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

void SpriteRenderer::init()
{
}

void SpriteRenderer::update(const float& dt, const float& elapsedTime)
{
    this->m_sprite.setTextureRect(this->m_rect);
}

void SpriteRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(this->m_sprite, states);
}

void SpriteRenderer::copy(std::shared_ptr<Component> otherCompPtr)
{
    Component::copy(otherCompPtr);

    std::shared_ptr<SpriteRenderer> other = std::dynamic_pointer_cast<SpriteRenderer>(otherCompPtr);
    this->m_texture = other->m_texture;

    if (other->m_rect.width < 0 || other->m_rect.height < 0) {
        this->m_rect = sf::IntRect(
            0, 0, 
            this->m_texture->getSize().x,
            this->m_texture->getSize().y
        );
    }
    else {
        this->m_rect = other->m_rect;
    }

    this->m_sprite = other->m_sprite;
}

void SpriteRenderer::loadDefinition(nlohmann::json object)
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

    if (this->m_rect.width == -1 || this->m_rect.height == -1)
    {
        this->m_texture = std::make_shared<sf::Texture>();
        this->m_texture->loadFromFile(this->m_properties.at("texture"));
    }
    else {
        this->m_texture = std::make_shared<sf::Texture>();
        this->m_texture->loadFromFile(this->m_properties.at("texture"), this->m_rect);
    }


    this->m_sprite = sf::Sprite();
    this->m_sprite.setTexture(*this->m_texture);
    this->m_sprite.setColor(sf::Color(255, 255, 255));
}
