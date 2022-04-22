#pragma once
#include <map>
#include <string>
#include <memory>

#include "Component.h"
#include "AnimationController.h"
#include "CharacterController.h"
#include "SpriteRenderer.h"
#include "PhysicsBody.h"
#include "TextRenderer.h"
#include "EngineStatsText.h"
#include "RocketLauncher.h"
#include "Minigun.h"
#include "ParticleSystem.h"
#include "RocketProjectile.h"
#include "Bullet.h"
#include "TimedDestroy.h"
#include "StatsPanel.h"
#include "Menu.h"

#include "PlayerDamageHandler.h"
#include "PropDamageHandler.h"

#include "Spike.h"

namespace PlatformDataEngine
{

    /// <summary>
    /// COMPONENT FACTORY
    /// This is a class that is responsible for creating components
    /// of different types from a string representation of the type.
    /// For example, if the component with type "AnimationController"
    /// is added in the json file, we need a way to make an object of
    /// type AnimationController so we'd call the following:
    /// ComponentFactory::create("AnimationController")
    /// </summary>
    class ComponentFactory
    {

        typedef std::map<std::string, std::shared_ptr<Component> (*)()> map_type;

    public:
        static inline std::shared_ptr<Component> create(const std::string &type)
        {
            return factoryMap.at(type)();
        }

    private:
        template <typename T>
        static inline std::shared_ptr<Component> construct()
        {
            return std::make_shared<T>();
        }

        static inline const map_type factoryMap = {
            {"AnimationController", &construct<AnimationController>},
            {"CharacterController", &construct<CharacterController>},
            {"SpriteRenderer", &construct<SpriteRenderer>},
            {"PhysicsBody", &construct<PhysicsBody>},
            {"TextRenderer", &construct<TextRenderer>},
            {"EngineStatsText", &construct<EngineStatsText>},
            {"RocketLauncher", &construct<RocketLauncher>},
            {"MiniGun", &construct<MiniGun>},
            {"ParticleSystem", &construct<ParticleSystem>},
            {"RocketProjectile", &construct<RocketProjectile>},
            {"Bullet", &construct<Bullet>},
            {"TimedDestroy", &construct<TimedDestroy>},
            {"StatsPanel", &construct<StatsPanel>},
            {"Menu", &construct<Menu>},
            {"Spike", &construct<Spike>},

            // damage handlers
            {"PlayerDamageHandler", &construct<PlayerDamageHandler>},
            {"PropDamageHandler", &construct<PropDamageHandler>}};
    };
}
