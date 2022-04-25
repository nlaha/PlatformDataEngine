#include "FloatingBox.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "PhysicsCallbacks.h"
#include "PlatformDataEngineWrapper.h"
#include "AnimationController.h"

using namespace PlatformDataEngine;

void FloatingBox::init()
{
    Component::init();
    PhysicsBody* pb = this->m_parent->findComponentOfType<PhysicsBody>().get();
    if (pb != nullptr) {
        this->m_PhysBody = pb;
    }
    else {
        spdlog::critical("GameObject {} has a Floating Box so it must also have a PhysicsBody", this->m_parent->getId());
    }
}

void FloatingBox::update(const float& dt, const float& elapsedTime) //Called once per update cycle
{
    if (std::abs(distance) >= 80) {
        direction = direction * -1; //This updates the direction if it has passed 5 lengths of the box in terms of distance
    }
    this->m_parent->move(((direction * 1)*dt), 0); //This creates horizonatal movment along the x-axis 
    distance = distance + (1 * dt);
}

void FloatingBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
}

void FloatingBox::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<FloatingBox> other = std::dynamic_pointer_cast<FloatingBox>(otherCompPtr);

    *this = *other;
}
void FloatingBox::loadDefinition(nlohmann::json object)
{
    this->direction = -1;
    this->distance = 0;
}