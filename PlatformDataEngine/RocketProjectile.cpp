#include "RocketProjectile.h"
#include "PhysicsBody.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;



RocketProjectile::~RocketProjectile()
{
}

void RocketProjectile::init()
{
    Component::init();

    if (this->m_sound != nullptr)
    {
        this->m_sound->setPosition(
            this->m_parent->getPosition().x,
            this->m_parent->getPosition().y,
            0.0f);
        this->m_sound->setAttenuation(0.3f);
        this->m_sound->play();
    }

    PhysicsBody* pb = this->m_parent->findComponentOfType<PhysicsBody>().get();

    if (pb != nullptr) {
        this->m_PhysBody = pb;

        b2Fixture* fix = this->m_PhysBody->getBody()->GetFixtureList();
        b2Filter filter;
        filter.categoryBits = PlatformDataEngine::PROJECTILE;
        fix->SetFilterData(filter);
    }
    else {
        spdlog::critical("GameObject {} has a RocketProjectile so it must also have a PhysicsBody", this->m_parent->getId());
    }
    this->m_PhysBody->getBody()->SetBullet(true);
    this->m_PhysBody->getBody()->SetGravityScale(0.0f);

    // set up force sensor fixture
    b2CircleShape forceSensorShape;
    forceSensorShape.m_radius = this->m_explosionRadius / Constants::PHYS_SCALE;
    b2FixtureDef forceSensorDef;
    forceSensorDef.shape = &forceSensorShape;
    forceSensorDef.isSensor = true;
    b2Filter forceSensorFilter;
    forceSensorFilter.categoryBits = PlatformDataEngine::WORLD_DYNAMIC;
    forceSensorDef.filter = forceSensorFilter;
    this->m_forceSensor = this->m_PhysBody->getBody()->CreateFixture(&forceSensorDef);

    // set up damage sensor fixture
    b2CircleShape damageSensorShape;
    damageSensorShape.m_radius = (this->m_explosionRadius / Constants::PHYS_SCALE) / 3.0f;
    b2FixtureDef damageSensorDef;
    damageSensorDef.shape = &damageSensorShape;
    damageSensorDef.isSensor = true;
    b2Filter damageSensorFilter;
    damageSensorFilter.categoryBits = PlatformDataEngine::WORLD_DYNAMIC;
    damageSensorDef.filter = damageSensorFilter;
    this->m_damageSensor = this->m_PhysBody->getBody()->CreateFixture(&damageSensorDef);

    this->m_isExploding = false;
}

void RocketProjectile::update(const float& dt, const float& elapsedTime)
{
    // only simulate on server
    if (!PlatformDataEngineWrapper::getIsClient()) {
        this->m_PhysBody->getBody()->SetLinearDamping(0);

        for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
        {
            if (c->contact->GetFixtureA()->IsSensor() != true &&
                c->contact->GetFixtureB()->IsSensor() != true) {
                this->m_isExploding = true;
                break;
            }
        }

        // we've hit something!
        if (this->m_isExploding) {

            sf::Vector2f ourPos = this->m_parent->getPosition();

            for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next)
            {
                // apply damage
                if (c->contact->GetFixtureA() == this->m_damageSensor || c->contact->GetFixtureB() == this->m_damageSensor) {

                    b2Body* body = c->other;

                    b2Vec2 bodyCenter = body->GetPosition();
                    float distFrac = std::fmaxf(0.0f, 1.10f - Utility::distance(bodyCenter, Utility::fromSf(ourPos)) / this->m_explosionRadius);
                    float velocityFalloff = std::sqrt(distFrac);

                    // don't apply damage to ourself
                    if (body->GetUserData().pointer != 0) {
                        if (reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer)->gameObjectOwner !=
                            this->m_owningGameObject.get())
                        {
                            // damage body
                            if (body->GetUserData().pointer != 0 &&
                                reinterpret_cast<PhysBodyUserData*>(
                                    body->GetUserData().pointer)->gameObjectOwner != nullptr)
                            {
                                reinterpret_cast<PhysBodyUserData*>(
                                    body->GetUserData().pointer)->gameObjectOwner->damage(this->m_explosionDamage * velocityFalloff);
                            }
                        }
                    }
                }

                // apply force
                if (c->contact->GetFixtureA() == this->m_forceSensor || c->contact->GetFixtureB() == this->m_forceSensor) {

                    b2Body* body = c->other;

                    // we don't need to do anything if we're interacting
                    // with a static body
                    if (body->GetType() == b2BodyType::b2_dynamicBody) {

                        b2Vec2 bodyCenter = body->GetPosition();
                        b2Vec2 impulseVec = Utility::normalize(b2Vec2(
                            (bodyCenter.x - (ourPos.x / Constants::PHYS_SCALE)),
                            (bodyCenter.y - (ourPos.y / Constants::PHYS_SCALE))
                        ));

                        float distFrac = std::fmaxf(0.0f, 1.10f - Utility::distance(bodyCenter, Utility::fromSf(ourPos)) / this->m_explosionRadius);
                        float velocityFalloff = std::sqrt(distFrac);

                        impulseVec.x *= this->m_explosionForce * velocityFalloff;
                        impulseVec.y *= this->m_explosionForce * velocityFalloff;

                        // don't apply force to particles
                        if (body->GetFixtureList()[0].GetFilterData().categoryBits != PhysicsCategory::PARTICLE) {
                            b2Vec2 velocity = body->GetLinearVelocity();
                            velocity.x *= 0.50f;
                            velocity.y *= 0.50f;
                            body->SetLinearVelocity(velocity);
                            body->ApplyLinearImpulseToCenter(impulseVec, true);
                        }
                    }
                }
            }

            // explode FX
            // spawn particle system
            PlatformDataEngineWrapper::getWorld()->spawnGameObject(
                this->m_ParticleSystemName, ourPos, "", false);

            // destroy self
            this->m_parent->destroySelf();
        }

        this->m_isExploding = false;
    }
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

    if (object.count("sound") > 0)
    {
        this->m_sound = PlatformDataEngineWrapper::getAudioSystem()->getSound(object.at("sound"));
    }
}

void RocketProjectile::setOwner(std::shared_ptr<GameObject> owner)
{
    this->m_owningGameObject = owner;
    reinterpret_cast<PhysBodyUserData*>(
        this->m_PhysBody->getBody()->GetUserData().pointer)->gameObjectOwner = this->m_owningGameObject.get();
}
