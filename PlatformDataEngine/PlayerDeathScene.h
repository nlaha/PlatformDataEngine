#pragma once
#include "GameScene.h"

namespace PlatformDataEngine {
    class PlayerDeathScene :
        public GameScene
    {
    public:
        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    };
}

