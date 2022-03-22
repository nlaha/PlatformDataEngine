#include "CharacterController.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

void PlatformDataEngine::CharacterController::init()
{
}

void PlatformDataEngine::CharacterController::update(const float& dt, const float& elapsedTime)
{
    std::shared_ptr<PhysicsBody> pb = this->m_parent->findComponentOfType<PhysicsBody>();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        // move left
        if (pb->getBody()->GetLinearVelocity().Length() < this->m_maxVelocity)
            pb->getBody()->ApplyForceToCenter({ -1.f * this->m_moveForce, 0.f }, true);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        // move right
        if (pb->getBody()->GetLinearVelocity().Length() < this->m_maxVelocity)
            pb->getBody()->ApplyForceToCenter({ 1.f * this->m_moveForce, 0.f }, true);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !this->m_prev_key_state)
    {
        // jump
        if (pb->getBody()->GetLinearVelocity().Length() < this->m_maxVelocity)
            pb->getBody()->ApplyLinearImpulseToCenter({ 0.f, -1.f * this->m_jumpForce }, true);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        // move down
        if (pb->getBody()->GetLinearVelocity().Length() < this->m_maxVelocity)
            pb->getBody()->ApplyForceToCenter({ 0.f, 1.f * this->m_moveForce }, true);
    }

    this->m_prev_key_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    // lerp velocity to zero
    b2Vec2 currentVelocity = pb->getBody()->GetLinearVelocity();
    b2Vec2 newVelocity = Utility::lerp(currentVelocity, { 0.f, currentVelocity.y }, 2.0f * dt);
    pb->getBody()->SetLinearVelocity(newVelocity);
}

void PlatformDataEngine::CharacterController::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= this->m_parent->getTransform();
}

void PlatformDataEngine::CharacterController::loadDefinition(nlohmann::json object)
{
    this->m_moveForce = object.at("moveForce");
    this->m_jumpForce = object.at("jumpForce");
    this->m_maxVelocity = object.at("maxVelocity");
}
