#include "CharacterController.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "PlatformDataEngineWrapper.h"
#include <spdlog/spdlog.h>

void PlatformDataEngine::CharacterController::init()
{
    std::shared_ptr<PhysicsBody> pb = this->m_parent->findComponentOfType<PhysicsBody>();
    if (pb.get() != nullptr) {
        this->m_pBody = pb;
    }
    else {
        spdlog::critical("GameObject {} has a CharacterController so it must also have a PhysicsBody", this->m_parent->getName());
    }
}

void PlatformDataEngine::CharacterController::update(const float& dt, const float& elapsedTime)
{
    b2Vec2 vel = this->m_pBody->getBody()->GetLinearVelocity();

    this->m_pBody->getBody()->SetAwake(true);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        // move left
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_pBody->getBody()->ApplyForceToCenter({ -1.f * this->m_moveForce, 0.f }, true);

        // sticking to walls
        if (!HasFlag(this->isAdjacentGround(), GroundTestMask::DOWN) &&
             HasFlag(this->isAdjacentGround(), GroundTestMask::LEFT) && vel.LengthSquared() < 10) {
            this->m_pBody->getBody()->SetAwake(false);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        // move right
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_pBody->getBody()->ApplyForceToCenter({ 1.f * this->m_moveForce, 0.f }, true);

        // sticking to walls
        if (!HasFlag(this->isAdjacentGround(), GroundTestMask::DOWN) &&
             HasFlag(this->isAdjacentGround(), GroundTestMask::RIGHT) && vel.LengthSquared() < 10) {
            this->m_pBody->getBody()->SetAwake(false);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
        !this->m_prev_key_state && 
        this->isAdjacentGround() != GroundTestMask::NONE)
    {
        this->m_pBody->getBody()->SetAwake(true);

        // jump
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_pBody->getBody()->ApplyLinearImpulseToCenter({ 0.f, -1.f * this->m_jumpForce }, true);
    }

    this->m_prev_key_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    // lerp velocity to zero
    b2Vec2 currentVelocity = this->m_pBody->getBody()->GetLinearVelocity();
    b2Vec2 newVelocity = Utility::lerp(currentVelocity, { 0.f, currentVelocity.y }, 4.0f * dt);
    this->m_pBody->getBody()->SetLinearVelocity(newVelocity);
}

void PlatformDataEngine::CharacterController::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PlatformDataEngine::CharacterController::loadDefinition(nlohmann::json object)
{
    this->m_moveForce = object.at("moveForce");
    this->m_jumpForce = object.at("jumpForce");
    this->m_maxVelocity = object.at("maxVelocity");
}

/// <summary>
/// Checks if the character is adjacent to the 
/// ground or some other solid object
/// </summary>
/// <returns></returns>
int PlatformDataEngine::CharacterController::isAdjacentGround() const
{
    // raycast output
    b2RayCastOutput castOutput = {};

    b2Vec2 startVec = this->m_pBody->getBody()->GetTransform().p;
    startVec.x += this->m_pBody->getBounds().width / 2.0f;
    startVec.y += this->m_pBody->getBounds().height / 2.0f;

    b2Vec2 downVec(0, this->m_pBody->getBounds().height);
    b2Vec2 leftVec(-this->m_pBody->getBounds().width, 0);
    b2Vec2 leftDownVec(-this->m_pBody->getBounds().width, 
        this->m_pBody->getBounds().height);

    b2Vec2 rightVec(this->m_pBody->getBounds().width, 0);
    b2Vec2 rightDownVec(this->m_pBody->getBounds().width, 
        this->m_pBody->getBounds().height);

    std::array<std::pair<int, b2Vec2>, 6> directions = {
        std::pair{DOWN, downVec},
        {LEFT, leftVec},
        {DOWNLEFT, leftDownVec },
        {RIGHT, rightVec},
        {DOWNRIGHT, rightDownVec},
        {NONE, b2Vec2(0.354, 0.354)}
    };

    int flags = GroundTestMask::NONE;
    std::find_if(std::execution::seq, directions.begin(), directions.end(), [&](const std::pair<int, b2Vec2>& dir) {
        RaycastCallback callback;
        PlatformDataEngineWrapper::getWorld()->getPhysWorld()->RayCast(&callback, startVec, startVec + dir.second);
        if (callback.m_fixture != nullptr) {
            flags = flags | dir.first;
        }
        return false;
    });

    return flags;

}
