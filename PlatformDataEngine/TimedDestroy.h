#pragma once
#include "Component.h"

namespace PlatformDataEngine {

    class TimedDestroy :
        public Component
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

    private:

        sf::Clock m_timer;
        float m_time;
        bool m_useRange;
        float m_timeLow;
        float m_timeHigh;
    };
}

