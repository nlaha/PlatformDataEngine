#include "PhysicsCallbacks.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

bool ContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
    const b2Filter& filterA = fixtureA->GetFilterData();
    const b2Filter& filterB = fixtureB->GetFilterData();

    if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0)
    {
        return filterA.groupIndex > 0;
    }

    bool collideA = (filterA.maskBits & filterB.categoryBits) != 0;
    bool collideB = (filterA.categoryBits & filterB.maskBits) != 0;
    bool collide = collideA && collideB;

    if (filterA.categoryBits == PlatformDataEngine::PROJECTILE || filterB.categoryBits == PlatformDataEngine::PROJECTILE) {
        if (fixtureA->GetBody()->GetUserData().pointer != 0 &&
            fixtureB->GetBody()->GetUserData().pointer != 0)
        {
            //spdlog::info("Data present in collision A: {} B: {}", fixtureA->GetBody()->GetUserData().pointer, fixtureB->GetBody()->GetUserData().pointer);

            PhysBodyUserData* bA = reinterpret_cast<PhysBodyUserData*>(fixtureA->GetBody()->GetUserData().pointer);
            PhysBodyUserData* bB = reinterpret_cast<PhysBodyUserData*>(fixtureB->GetBody()->GetUserData().pointer);

            //spdlog::info("Collision for:");
            //spdlog::info("A: {}", fmt::ptr(bA->gameObjectOwner));
            //spdlog::info("B: {}", fmt::ptr(bB->gameObjectOwner));
            if (bB != nullptr && bA != nullptr) {
                if (bA->gameObjectOwner != nullptr && bB->gameObjectOwner != nullptr) {
                    //spdlog::info("Checking collision for {} and {}", fmt::ptr(bA->gameObjectOwner), fmt::ptr(bB->gameObjectOwner));
                    if (bA->gameObjectOwner == bB->gameObjectOwner)
                    {
                        //spdlog::info("Ignoring collision for {} and {}", fmt::ptr(bA->gameObjectOwner), fmt::ptr(bB->gameObjectOwner));
                        collide = false;
                    }
                }
            }
        }
    }

    return collide;
}
