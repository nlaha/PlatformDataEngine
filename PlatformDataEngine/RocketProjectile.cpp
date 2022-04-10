#include "RocketProjectile.h"
#include "PhysicsBody.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;



void RocketProjectile::init()
{
    Component::init();

    std::shared_ptr<PhysicsBody> pb = this->m_parent->findComponentOfType<PhysicsBody>();
    if (pb.get() != nullptr) {
        this->m_PhysBody = pb;

        b2Fixture* fix = this->m_PhysBody->getBody()->GetFixtureList();
        b2Filter filter;
        filter.categoryBits = PlatformDataEngine::FRIEND_PROJECTILE;
        fix->SetFilterData(filter);
    }
    else {
        spdlog::critical("GameObject {} has a RocketProjectile so it must also have a PhysicsBody", this->m_parent->getName());
    }
    this->m_PhysBody->getBody()->SetBullet(true);
    this->m_PhysBody->getBody()->SetGravityScale(0.0f);

    b2CircleShape circleShape;
    circleShape.m_radius = this->m_explosionRadius;
    b2FixtureDef sensorDef;
    sensorDef.shape = &circleShape;
    sensorDef.isSensor = true;
    b2Filter filterSensor;
    filterSensor.categoryBits = PlatformDataEngine::WORLD_DYNAMIC;
    sensorDef.filter = filterSensor;
    this->m_explosionSensor = this->m_PhysBody->getBody()->CreateFixture(&sensorDef);

    this->m_isExploding = false;
}

void RocketProjectile::update(const float& dt, const float& elapsedTime)
{
    this->m_PhysBody->getBody()->SetLinearDamping(0);

    for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
    {
        if (c->contact->GetFixtureA()->IsSensor() != true &&
            c->contact->GetFixtureB()->IsSensor() != true) {
            // don't explode on self
            if (c->other->GetUserData().pointer != 0) {
                GameObject* otherGameObject = reinterpret_cast<PhysBodyUserData*>(c->other->GetUserData().pointer)->gameObjectOwner;
                std::string player = PlatformDataEngineWrapper::getWorld()->getPlayer()->getName();
                if (otherGameObject->getName() != player)
                {
                    this->m_isExploding = true;
                    break;
                }
            }
            else {
                this->m_isExploding = true;
                break;
            }
        }
    }

    if (this->m_isExploding) {

        sf::Vector2f ourPos = this->m_parent->getPosition();

        // apply force
        for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
        {
            b2Body* body = c->other;

            if (body->GetType() == b2BodyType::b2_dynamicBody) {

                b2Vec2 bodyCenter = body->GetPosition();
                b2Vec2 impulseVec = Utility::normalize(b2Vec2(
                    (bodyCenter.x - ourPos.x),
                    (bodyCenter.y - ourPos.y)
                ));

                float distFrac = std::fmaxf(0.0f, 1.10f - Utility::distance(bodyCenter, Utility::fromSf(ourPos)) / this->m_explosionRadius);
                float velocityFalloff = std::sqrtf(distFrac);

                impulseVec.x *= this->m_explosionForce * velocityFalloff;
                impulseVec.y *= this->m_explosionForce * velocityFalloff;

                float friendlyFireMultiplier = 1.0f;
                if (reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer)->gameObjectOwner == 
                    PlatformDataEngineWrapper::getWorld()->getPlayer().get())
                {
                    friendlyFireMultiplier = 0.1f;
                }

                // damage body
                if (body->GetUserData().pointer != 0 &&
                    reinterpret_cast<PhysBodyUserData*>(
                        body->GetUserData().pointer)->gameObjectOwner != nullptr)
                {
                    reinterpret_cast<PhysBodyUserData*>(
                        body->GetUserData().pointer)->gameObjectOwner->damage(this->m_explosionDamage * velocityFalloff * friendlyFireMultiplier);
                }

                b2Vec2 velocity = body->GetLinearVelocity();
                velocity.x *= 0.50f;
                velocity.y *= 0.50f;
                body->SetLinearVelocity(velocity);
                body->ApplyLinearImpulseToCenter(impulseVec, true);
            }
        }

        // explode FX
        // spawn particle system
        PlatformDataEngineWrapper::getWorld()->spawnGameObject(
            this->m_ParticleSystemName, ourPos);

        // destroy self
        this->m_parent->destroySelf();
    }

    this->m_isExploding = false;
}

void RocketProjectile::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void RocketProjectile::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<RocketProjectile> other = std::dynamic_pointer_cast<RocketProjectile>(otherCompPtr);

    *this = *other;
}

void RocketProjectile::loadDefinition(nlohmann::json object)
{
    this->m_explosionDamage = object.at("explosionDamage");
    this->m_explosionForce = object.at("explosionForce");
    this->m_explosionRadius = object.at("explosionRadius");
    this->m_ParticleSystemName = object.at("particleSystemName");
}
