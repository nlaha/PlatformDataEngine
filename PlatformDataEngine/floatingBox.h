#pragma once
#include "Component.h"
#include <spdlog/spdlog.h>
namespace PlatformDataEngine
{
    class PhysicsBody; //Forward Declaration
    class FloatingBox : public Component {
    public:
        // called once when the game starts for every instance of the game object
        void init();

        // for updating things
        void update(const float& dt, const float& elapsedTime);

        // for drawing things (called on separate thread)
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        // YOU MUST IMPLEMENT COPY (BAD THINGS WILL HAPPEN IF YOU DON'T)
        // like a copy constructor but you must std::dynamic_pointer_cast the parameter
        void copy(std::shared_ptr<Component> otherCompPtr);

        // called with the "properties" object in a game object json file, 
        // called once for the definition (not called on instances)
        void loadDefinition(nlohmann::json object);

    private:
        PhysicsBody* m_PhysBody;
        float distance;
        int direction;
    };
}
