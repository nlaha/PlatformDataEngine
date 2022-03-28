#include "CharacterController.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "AnimationController.h"
#include "PlatformDataEngineWrapper.h"
#include "TileMap.h"
#include <spdlog/spdlog.h>

void PlatformDataEngine::CharacterController::init()
{
    std::shared_ptr<PhysicsBody> pb = this->m_parent->findComponentOfType<PhysicsBody>();
    if (pb.get() != nullptr) {
        this->m_PhysBody = pb;
    }
    else {
        spdlog::critical("GameObject {} has a CharacterController so it must also have a PhysicsBody", this->m_parent->getName());
    }

    std::shared_ptr<AnimationController> animController = this->m_parent->findComponentOfType<AnimationController>();
    if (animController.get() != nullptr) {
        this->m_AnimController = animController;
    }
    else {
        spdlog::critical("GameObject {} has a CharacterController so it must also have a AnimationController", this->m_parent->getName());
    }

    this->m_pInputManager = PlatformDataEngineWrapper::getPlayerInputManager();
}

void PlatformDataEngine::CharacterController::update(const float& dt, const float& elapsedTime)
{
    b2Vec2 vel = this->m_PhysBody->getBody()->GetLinearVelocity();

    this->m_PhysBody->getBody()->SetAwake(true);

    this->m_AnimController->setAnimation("Idle", 1.0f, true);

    float axisModifier = 1.0f;
    if (m_pInputManager->getAxis("x").isNegative())
    {
        axisModifier = std::abs(m_pInputManager->getAxis("x").getValue() / 100.0f);
        // move left
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyForceToCenter({ -1.f * this->m_moveForce, 0.f }, true);

        this->m_AnimController->setFlipFlag(AnimationController::FlipFlags::HORIZONTAL);
        this->m_AnimController->setAnimation("Walk", 4.0f * axisModifier, true);
    }

    if (m_pInputManager->getAxis("x").isPositive())
    {
        axisModifier = std::abs(m_pInputManager->getAxis("x").getValue() / 100.0f);
        // move right
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyForceToCenter(
                { 1.f * this->m_moveForce * axisModifier, 0.f}, true);

        this->m_AnimController->setFlipFlag(AnimationController::FlipFlags::NONE);
        this->m_AnimController->setAnimation("Walk", 4.0f * axisModifier, true);
    }

    if (m_pInputManager->getButton("jump").getValue() &&
        !this->m_prev_jump_state && 
        this->fastGroundCheck() && this->m_jumpCooldownClock.getElapsedTime().asMilliseconds() > this->m_jumpCooldown)
    {
        this->m_PhysBody->getBody()->SetAwake(true);

        // jump
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyLinearImpulseToCenter({ 0.f, -1.f * this->m_jumpForce }, true);
        m_jumpCooldownClock.restart();
    }

    this->m_prev_jump_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Joystick::isButtonPressed(0, 1);

    if (m_pInputManager->getButton("dash").getValue() &&
        this->m_prev_dash_state &&
        this->fastGroundCheck() && this->m_dashCooldownClock.getElapsedTime().asMilliseconds() > this->m_dashCooldown)
    {
        this->m_PhysBody->getBody()->SetAwake(true);

        // dash
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyLinearImpulseToCenter({ (vel.x > 0 ? 1 : -1) * this->m_jumpForce, 0.0 }, true);
        m_dashCooldownClock.restart();
    }

    this->m_prev_dash_state = this->fastGroundCheck();

    // lerp velocity to zero
    b2Vec2 currentVelocity = this->m_PhysBody->getBody()->GetLinearVelocity();
    b2Vec2 newVelocity = Utility::lerp(currentVelocity, { 0.f, currentVelocity.y }, 4.0f * dt);
    this->m_PhysBody->getBody()->SetLinearVelocity(newVelocity);

    for (TileMap::TilesetPair& ts : PlatformDataEngineWrapper::getWorld()->getTileMap()->getTilesets())
    {
        ts.tileset->getShader()->setUniform("charPos", this->m_parent->getPosition());
    }

    // update animation state
    this->updateAnimation(vel);
}

void PlatformDataEngine::CharacterController::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PlatformDataEngine::CharacterController::loadDefinition(nlohmann::json object)
{
    this->m_moveForce = object.at("moveForce");
    this->m_jumpForce = object.at("jumpForce");
    this->m_maxVelocity = object.at("maxVelocity");
    this->m_jumpCooldown = object.at("jumpCooldown");
    this->m_dashCooldown = object.at("dashCooldown");
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

    b2Vec2 startVec = this->m_PhysBody->getBody()->GetTransform().p;
    startVec.x += this->m_PhysBody->getBounds().width / 2.0f;
    startVec.y += this->m_PhysBody->getBounds().height / 2.0f;

    b2Vec2 downVec(0, this->m_PhysBody->getBounds().height);
    b2Vec2 leftVec(-this->m_PhysBody->getBounds().width, 0);
    b2Vec2 leftDownVec(-this->m_PhysBody->getBounds().width, 
        this->m_PhysBody->getBounds().height);

    b2Vec2 rightVec(this->m_PhysBody->getBounds().width, 0);
    b2Vec2 rightDownVec(this->m_PhysBody->getBounds().width, 
        this->m_PhysBody->getBounds().height);

    std::array<std::pair<int, b2Vec2>, 6> directions = {
        std::pair{DOWN, downVec},
        {LEFT, leftVec},
        {DOWNLEFT, leftDownVec },
        {RIGHT, rightVec},
        {DOWNRIGHT, rightDownVec},
        {NONE, b2Vec2(0.0, 0.0)}
    };

    int flags = GroundTestMask::NONE;
    std::find_if(std::execution::seq, directions.begin(), directions.end(), [&](const std::pair<int, b2Vec2>& dir) {
        RaycastCallback callback;
        PlatformDataEngineWrapper::getWorld()->getPhysWorld()->RayCast(&callback, startVec, startVec + dir.second);
        if (callback.m_fixture != nullptr) {
            // disable self raycast collision
            if (!callback.isBody(this->m_PhysBody->getBody())) {
                flags = flags | dir.first;
            }
        }
        return false;
    });

    return flags;

}

/// <summary>
/// Use this if you don't need to know the direction the player
/// is contacting the ground
/// </summary>
/// <returns></returns>
bool PlatformDataEngine::CharacterController::fastGroundCheck() const
{
    for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
    {
        return true;
    }

    return false;
}

void PlatformDataEngine::CharacterController::updateAnimation(b2Vec2 velocity)
{
    if (this->fastGroundCheck()) {
        // on ground
    }
    else {
        this->m_AnimController->setAnimation("InAir", 2.0f, true);
    }
}
