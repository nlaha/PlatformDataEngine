#include "PhysicsBody.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

PlatformDataEngine::PhysicsBody::~PhysicsBody()
{
}

void PlatformDataEngine::PhysicsBody::init()
{
	b2BodyDef bodyDef;
	bodyDef.fixedRotation = true;
	bodyDef.type = this->m_bodyType;

	sf::Vector2f initPos = this->m_parent->getPosition();
	bodyDef.position.Set(initPos.x, initPos.y);

	this->m_body = PlatformDataEngineWrapper::getWorld()->getPhysWorld()->CreateBody(&bodyDef);
	if (this->m_body != nullptr) {

		this->m_bodyShape = std::make_shared<b2PolygonShape>();
		std::array<b2Vec2, 4> pointArr = {
			b2Vec2(this->m_bounds.left, this->m_bounds.top),
			b2Vec2(this->m_bounds.left + this->m_bounds.width, this->m_bounds.top),
			b2Vec2(this->m_bounds.left + this->m_bounds.width, this->m_bounds.top + this->m_bounds.height),
			b2Vec2(this->m_bounds.left, this->m_bounds.top + this->m_bounds.height),
		};
		this->m_bodyShape->Set(pointArr.data(), pointArr.size());

		this->m_bodyFixtureDef = std::make_shared<b2FixtureDef>();
		this->m_bodyFixtureDef->shape = m_bodyShape.get();
		this->m_bodyFixtureDef->density = this->m_density;
		this->m_bodyFixtureDef->friction = this->m_friction;
		this->m_bodyFixtureDef->restitution = this->m_bouncy;

		this->m_body->CreateFixture(this->m_bodyFixtureDef.get());
	}
}

void PlatformDataEngine::PhysicsBody::update(const float& dt, const float& elapsedTime)
{
	this->m_parent->setPosition(
		this->m_body->GetTransform().p.x,
		this->m_body->GetTransform().p.y
	);
}

void PlatformDataEngine::PhysicsBody::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PlatformDataEngine::PhysicsBody::loadDefinition(nlohmann::json object)
{
	this->m_bounds = sf::FloatRect(
		object.at("rect").at("x"),
		object.at("rect").at("y"),
		object.at("rect").at("width"),
		object.at("rect").at("height")
	);

	this->m_bodyType = object.at("type") == "dynamic" ? b2_dynamicBody : b2_staticBody;

	this->m_density = object.at("density");
	this->m_friction = object.at("friction");
	this->m_bouncy = object.at("bouncy");

}
