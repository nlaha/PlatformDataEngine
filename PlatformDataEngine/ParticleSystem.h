#pragma once
#include <thread>
#include <mutex>
#include <SFML/Audio.hpp>
#include "Component.h"

namespace PlatformDataEngine {

    class GameObject;

    /// <summary>
    /// The particle system component spawns a number
    /// of other game objects and applies a velocity to them
    /// </summary>
    class ParticleSystem :
        public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

    private:

        void spawnParticles();

        std::string m_particleName;
        int m_numParticles;
        float m_velocity;
        sf::Sound* m_sound;
    };
}

