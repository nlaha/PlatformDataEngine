#include "Bullet.h"
#include "PhysicsBody.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;



Bullet::~Bullet()
{
}

void Bullet::init()
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
        spdlog::critical("GameObject {} has a Bullet so it must also have a PhysicsBody", this->m_parent->getId());
    }
    this->m_PhysBody->getBody()->SetBullet(true);
    this->m_PhysBody->getBody()->SetGravityScale(0.0f);

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

void Bullet::update(const float& dt, const float& elapsedTime)
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

void Bullet::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void Bullet::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<Bullet> other = std::dynamic_pointer_cast<Bullet>(otherCompPtr);

    *this = *other;
}

void Bullet::loadDefinition(nlohmann::json object)
{
    this->m_explosionDamage = object.at("explosionDamage");
    //this->m_explosionForce = object.at("explosionForce");
    this->m_explosionRadius = object.at("explosionRadius");
    this->m_ParticleSystemName = object.at("particleSystemName");

    if (object.count("sound") > 0)
    {
        this->m_sound = PlatformDataEngineWrapper::getAudioSystem()->getSound(object.at("sound"));
    }
}

void Bullet::setOwner(std::shared_ptr<GameObject> owner)
{
    this->m_owningGameObject = owner;
    reinterpret_cast<PhysBodyUserData*>(
        this->m_PhysBody->getBody()->GetUserData().pointer)->gameObjectOwner = this->m_owningGameObject.get();
}