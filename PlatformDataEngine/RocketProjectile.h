#pragma once
#include <memory>
#include <box2d/box2d.h>

#include "Component.h"
#include "Utility.h"

namespace PlatformDataEngine {

    class PhysicsBody;

    /// <summary>
    /// The rocket launcher rocket projectile
    /// TODO: make this into a more generic "projectile" class
    /// with, rocket, grenade, bullet, beam, etc. subclasses
    /// </summary>
    class RocketProjectile :
        public Component
    {
    public:

        ~RocketProjectile();

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void setOwner(std::shared_ptr<GameObject> owner);

    private:

        std::shared_ptr<GameObject> m_owningGameObject;

        PhysicsBody* m_PhysBody;
        b2Fixture* m_explosionSensor;

        std::string m_ParticleSystemName;
        float m_explosionRadius;
        float m_explosionForce;
        float m_explosionDamage;

        bool m_isExploding;
    };
}

