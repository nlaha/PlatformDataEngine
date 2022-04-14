#include "CharacterController.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "AnimationController.h"
#include "PlatformDataEngineWrapper.h"
#include "TileMap.h"
#include "Server.h"
#include "InputManager.h"
#include <spdlog/spdlog.h>

using namespace PlatformDataEngine;

void CharacterController::init()
{
    Component::init();

    PhysicsBody* pb = this->m_parent->findComponentOfType<PhysicsBody>().get();
    if (pb != nullptr) {
        this->m_PhysBody = pb;

        b2Fixture* fix = this->m_PhysBody->getBody()->GetFixtureList();
        while (fix != nullptr)
        {

            b2Filter filter;
            filter.categoryBits = PlatformDataEngine::CHARACTER;
            filter.maskBits =
                PlatformDataEngine::CHARACTER |
                PlatformDataEngine::WORLD_DYNAMIC |
                PlatformDataEngine::WORLD_STATIC |
                PlatformDataEngine::PROJECTILE;
            fix->SetFilterData(filter);

            fix = fix->GetNext();
        }
    }
    else {
        spdlog::critical("GameObject {} has a CharacterController so it must also have a PhysicsBody", this->m_parent->getName());
    }

    AnimationController* animController = this->m_parent->findComponentOfType<AnimationController>().get();
    if (animController != nullptr) {
        this->m_AnimController = animController;
    }
    else {
        spdlog::critical("GameObject {} has a CharacterController so it must also have a AnimationController", this->m_parent->getName());
    }

    // attach player input manager if we're the player
    if (this->m_parent == PlatformDataEngineWrapper::getWorld()->getPlayer()) {
        this->m_pInputManager = PlatformDataEngineWrapper::getPlayerInputManager();
    }
    else {
        this->m_pInputManager = std::make_shared<NetworkInputManager>();
        std::shared_ptr<NetworkInputManager> netIn = std::dynamic_pointer_cast<NetworkInputManager>(this->m_pInputManager);
        netIn->loadDefinition("./game/input.json");
        if (!PlatformDataEngineWrapper::getIsClient()) {
            dynamic_cast<Server*>(PlatformDataEngineWrapper::getNetworkHandler())->addInputManager(this->m_parent->getConnection(), netIn);
        }
    }
}

void CharacterController::update(const float& dt, const float& elapsedTime)
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
            this->m_PhysBody->getBody()->ApplyForceToCenter({ -1.f * this->m_moveForce * axisModifier, 0.f }, true);

        this->m_AnimController->setFlipFlag(AnimationController::FlipFlags::HORIZONTAL);
        this->m_AnimController->setAnimation("Walk", 4.0f * std::fabsf(axisModifier), true);
    }

    if (m_pInputManager->getAxis("x").isPositive())
    {
        axisModifier = std::abs(m_pInputManager->getAxis("x").getValue() / 100.0f);
        // move right
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyForceToCenter(
                { 1.f * this->m_moveForce * axisModifier, 0.f}, true);

        this->m_AnimController->setFlipFlag(AnimationController::FlipFlags::NONE);
        this->m_AnimController->setAnimation("Walk", 4.0f * std::fabsf(axisModifier), true);
    }

    if (m_pInputManager->getButton("jump").getValue() &&
        !this->m_prev_jump_state && 
        this->fastGroundCheck() && this->m_jumpCooldownClock.getElapsedTime().asMilliseconds() > this->m_jumpCooldown)
    {
        this->m_PhysBody->getBody()->SetAwake(true);
        
        b2Vec2 wallJumpBoost = { 0.0f, 1.0f };
        if (isAdjacentWall() == DirTestMask::LEFT)
        {
            wallJumpBoost = { 0.5f, 1.25f };

        }
        else if (isAdjacentWall() == DirTestMask::RIGHT)
        {
            wallJumpBoost = { -0.5f, 1.25f };
        }

        // jump
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyLinearImpulseToCenter((b2Vec2{ wallJumpBoost.x * this->m_jumpForce , -1.f * this->m_jumpForce * wallJumpBoost.y }), true);
        m_jumpCooldownClock.restart();
    }

    this->m_prev_jump_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Joystick::isButtonPressed(0, 1);

    if (m_pInputManager->getButton("dash").getValue() &&
        this->m_prev_dash_state && this->m_dashCooldownClock.getElapsedTime().asMilliseconds() > this->m_dashCooldown)
    {
        this->m_PhysBody->getBody()->SetAwake(true);

        // dash
        if (vel.LengthSquared() <= this->m_maxVelocity)
            this->m_PhysBody->getBody()->ApplyLinearImpulseToCenter({ (vel.x > 0 ? 1 : -1) * this->m_jumpForce * 2, 0.0 }, true);
        m_dashCooldownClock.restart();
    }

    this->m_prev_dash_state = !this->fastGroundCheck();

    // lerp velocity to zero
    b2Vec2 currentVelocity = this->m_PhysBody->getBody()->GetLinearVelocity();
    b2Vec2 newVelocity = Utility::lerp(currentVelocity, { 0.f, currentVelocity.y }, 4.0f * dt);
    this->m_PhysBody->getBody()->SetLinearVelocity(newVelocity);
    this->m_PhysBody->getBody()->SetLinearDamping(0.25f);

    for (TileMap::TilesetPair& ts : PlatformDataEngineWrapper::getWorld()->getTileMap()->getTilesets())
    {
        ts.tileset->getShader()->setUniform("charPos", this->m_parent->getPosition());
    }

    // update animation state
    this->updateAnimation(vel);
}

void CharacterController::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void CharacterController::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<CharacterController> other = std::dynamic_pointer_cast<CharacterController>(otherCompPtr);

    *this = *other;
}

void CharacterController::loadDefinition(nlohmann::json object)
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
DirTestMask CharacterController::isAdjacentWall() const
{
    DirTestMask flags = DirTestMask::NONE;
    for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
    {
        // find direction of contact
        b2WorldManifold worldManifold;
        c->contact->GetWorldManifold(&worldManifold); // this method calls b2WorldManifold::Initialize with the appropriate transforms and radii so you don't have to worry about that
        b2Vec2 worldNormal = worldManifold.normal;

        if (worldNormal.x > 0 && worldNormal.y == 0) {
            flags = DirTestMask::LEFT;
        }
        else if (worldNormal.x < 0 && worldNormal.y == 0) {
            flags = DirTestMask::RIGHT;
        }
    }

    return flags;
}

/// <summary>
/// Use this if you don't need to know the direction the player
/// is contacting the ground
/// </summary>
/// <returns></returns>
bool CharacterController::fastGroundCheck() const
{
    for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
    {
        return true;
    }

    return false;
}

void CharacterController::updateAnimation(b2Vec2 velocity)
{
    if (this->fastGroundCheck()) {
        // on ground
    }
    else {
        this->m_AnimController->setAnimation("InAir", 2.0f, true);
    }
}
