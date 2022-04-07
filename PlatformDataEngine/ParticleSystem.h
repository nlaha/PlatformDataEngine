#pragma once
#include <thread>
#include <mutex>
#include "Component.h"

namespace PlatformDataEngine {

    class GameObject;

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
    };
}

