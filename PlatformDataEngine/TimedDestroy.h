#pragma once
#include "Component.h"

namespace PlatformDataEngine {

    /// <summary>
    /// Destroys the game object after a certain time
    /// useful for particles
    /// </summary>
    class TimedDestroy :
        public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void networkSerialize(PDEPacket& output);
        void networkDeserialize(PDEPacket& input);

    private:

        sf::Clock m_timer;
        float m_time;
        bool m_useRange;
        float m_timeLow;
        float m_timeHigh;
    };
}

