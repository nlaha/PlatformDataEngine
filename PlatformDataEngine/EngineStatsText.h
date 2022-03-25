#pragma once
#include "TextRenderer.h"
namespace PlatformDataEngine {

    class EngineStatsText :
        public TextRenderer
    {
    public:
        void update(const float& dt, const float& elapsedTime);

    };
}
