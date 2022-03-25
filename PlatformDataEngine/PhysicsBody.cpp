#include "PhysicsBody.h"
#include "GameObject.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

PlatformDataEngine::PhysicsBody::~PhysicsBody()
{
}

void PlatformDataEngine::PhysicsBody::init()
{
	sf::Vector2f initPos = this->m_parent->getPosition();
	this->getBody()->SetTransform({initPos.x, initPos.y}, 0.0);
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
	this->m_bodyType = object.at("type") == "dynamic" ? b2_dynamicBody : b2_staticBody;

	this->m_density = object.at("density");
	this->m_friction = object.at("friction");
	this->m_bouncy = object.at("bouncy");

	b2BodyDef bodyDef;
	bodyDef.fixedRotation = true;
	bodyDef.type = this->m_bodyType;
	bodyDef.position.Set(0, 0);

	this->m_body = PlatformDataEngineWrapper::getWorld()->getPhysWorld()->CreateBody(&bodyDef);
	if (this->m_body != nullptr) {
		for (const nlohmann::json& fixture : object.at("fixtures"))
		{
			sf::FloatRect bounds = sf::FloatRect(
				fixture.at("rect").at("x"),
				fixture.at("rect").at("y"),
				fixture.at("rect").at("width"),
				fixture.at("rect").at("height")
			);

			std::shared_ptr<b2FixtureDef> bodyFixtureDef = std::make_shared<b2FixtureDef>();
			bodyFixtureDef->density = this->m_density;
			bodyFixtureDef->friction = this->m_friction;
			bodyFixtureDef->restitution = this->m_bouncy;

			if (fixture.at("shape") == "box") {
				std::shared_ptr<b2PolygonShape> shape = std::make_shared<b2PolygonShape>();
				std::array<b2Vec2, 4> pointArr = {
					b2Vec2(bounds.left, bounds.top),
					b2Vec2(bounds.left + bounds.width, bounds.top),
					b2Vec2(bounds.left + bounds.width, bounds.top + bounds.height),
					b2Vec2(bounds.left, bounds.top + bounds.height),
				};

				shape->Set(pointArr.data(), pointArr.size());
				bodyFixtureDef->shape = shape.get();
				this->m_body->CreateFixture(bodyFixtureDef.get());
			}
			else if (fixture.at("shape") == "circle") {
				std::shared_ptr<b2CircleShape> shape = std::make_shared<b2CircleShape>();
				shape->m_p.Set(bounds.left, bounds.top + bounds.height);
				shape->m_radius = bounds.width / 2.0f;
				bodyFixtureDef->shape = shape.get();
				this->m_body->CreateFixture(bodyFixtureDef.get());
			}
		}

		this->m_bounds = sf::FloatRect(
			object.at("rect").at("x"),
			object.at("rect").at("y"),
			object.at("rect").at("width"),
			object.at("rect").at("height")
		);
	}
}
