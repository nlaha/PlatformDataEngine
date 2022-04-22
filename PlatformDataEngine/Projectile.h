#pragma once
#include <memory>
#include <box2d/box2d.h>
#include <SFML/Audio.hpp>

#include "Component.h"
#include "Utility.h"

namespace PlatformDataEngine {

    class PhysicsBody;

    /// <summary>
    /// The projectile generic class
    /// </summary>
    class Projectile :
        public Component
    {
    public:

        ~Projectile();

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void setOwner(std::shared_ptr<GameObject> owner);

    protected:

        std::shared_ptr<GameObject> m_owningGameObject;

        PhysicsBody* m_PhysBody;
        b2Fixture* m_forceSensor;
        b2Fixture* m_damageSensor;

        std::string m_ParticleSystemName;
        float m_explosionRadius;
        float m_explosionForce;
        float m_explosionDamage;

        sf::Sound* m_sound;

        bool m_isExploding;
    };
}