#include "Spike.h"
#include "GameObject.h"
#include "PhysicsBody.h"
#include "PhysicsCallbacks.h"
using namespace PlatformDataEngine;
void Spike::init()
{
    Component::init();
    PhysicsBody* pb = this->m_parent->findComponentOfType<PhysicsBody>().get();
    if (pb != nullptr) {
        this->m_PhysBody = pb;
    }
    else {
        spdlog::critical("GameObject {} has a Spike so it must also have a PhysicsBody", this->m_parent->getName());
    }
}

void Spike::update(const float& dt, const float& elapsedTime) //Called once per update cycle
{//Checks for collisions
	for (b2ContactEdge* c = this->m_PhysBody->getBody()->GetContactList(); c; c = c->next) {
        b2Body* body = c->other;
        if (body->GetUserData().pointer != 0) {
            PhysBodyUserData* userData = reinterpret_cast<PhysBodyUserData*>(body->GetUserData().pointer);
            userData->gameObjectOwner->damage(dt*10.0);
        }
	}
}

void Spike::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void Spike::copy(std::shared_ptr<Component> otherCompPtr)
{
}

void Spike::loadDefinition(nlohmann::json object)
{
}
