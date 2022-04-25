#pragma once
#include <memory>
#include <box2d/box2d.h>
#include <SFML/Audio.hpp>

#include "Component.h"
#include "Utility.h"
#include "Projectile.h"

namespace PlatformDataEngine {

    class PhysicsBody;

    /// <summary>
    /// The MiniGun bullet projectile
    /// </summary>
    class Bullet :
        public Projectile
    {
    public:

        ~Bullet();

        void init();

        void update(const float& dt, const float& elapsedTime);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void loadDefinition(nlohmann::json object);

        void setOwner(std::shared_ptr<GameObject> owner);


    private:

    };
}