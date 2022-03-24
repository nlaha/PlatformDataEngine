#include "AnimationController.h"
#include "SpriteRenderer.h"
#include "GameObject.h"
#include <spdlog/spdlog.h>

using namespace PlatformDataEngine;

void AnimationController::init()
{
    std::shared_ptr<SpriteRenderer> spriteRender = this->m_parent->findComponentOfType<SpriteRenderer>();
    if (spriteRender.get() != nullptr) {
        this->m_spriteRenderer = spriteRender;
    }
    else {
        spdlog::critical("GameObject {} has a AnimationController so it must also have a SpriteRenderer", this->m_parent->getName());
    }
}

void AnimationController::update(const float& dt, const float& elapsedTime)
{
    sf::FloatRect bounds = this->m_spriteRenderer->getSprite()->getLocalBounds();
    if (this->m_flip == AnimationController::FlipFlags::HORIZONTAL) {
        this->m_spriteRenderer->getSprite()->setScale({ -1.0, 1.0 });
        this->m_spriteRenderer->getSprite()->setPosition({ bounds.width, 0.0 });
    }

    if (this->m_flip == AnimationController::FlipFlags::VERTICAL) {
        this->m_spriteRenderer->getSprite()->setScale({ 1.0, -1.0 });
        this->m_spriteRenderer->getSprite()->setPosition({ 0.0, bounds.height });
    }

    if (this->m_flip == AnimationController::FlipFlags::NONE) {
        this->m_spriteRenderer->getSprite()->setScale({ 1.0, 1.0 });
        this->m_spriteRenderer->getSprite()->setPosition({ 0, 0.0 });
    }
}

void AnimationController::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void AnimationController::loadDefinition(nlohmann::json object)
{
    // TODO: load and parse aesprite file

    this->m_currentAnim = object.at("animation");
    this->m_loop = object.at("loop");
    this->m_speed = object.at("speed");
    this->m_flip = AnimationController::FlipFlags::HORIZONTAL;
}
