#pragma once
#include <memory>

#include "Component.h"
#include "PlayerInputManager.h"
#include "NetworkInputManager.h"
#include "Globals.h"
#include "Weapon.h"

namespace PlatformDataEngine {

    /// <summary>
    /// The rocket launcher component
    /// </summary>
    class RocketLauncher :
        public Weapon
    {
    public:
        void update(const float& dt, const float& elapsedTime);
        void copy(std::shared_ptr<Component> otherCompPtr);
   
    private:

    };
}

