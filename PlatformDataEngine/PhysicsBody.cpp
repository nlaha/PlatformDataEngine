#include "PhysicsBody.h"
#include "GameObject.h"
#include "Utility.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

PhysicsBody::PhysicsBody() : Component()
{
}

void PhysicsBody::copy(std::shared_ptr<Component> otherCompPtr)
{
	PhysicsBody* other = std::dynamic_pointer_cast<PhysicsBody>(otherCompPtr).get();

	this->m_bodyType = other->m_bodyType;

	this->m_density = other->m_density;
	this->m_friction = other->m_friction;
	this->m_bouncy = other->m_bouncy;
	this->m_doesRotate = other->m_doesRotate;
	this->m_bounds = other->m_bounds;
	this->m_bodyUserData = nullptr;
	this->m_isContinuous = other->m_isContinuous;

	this->m_body = PlatformDataEngineWrapper::getWorld()->getPhysWorld()->CreateBody(&(other->m_bodyDef));
	if (this->m_body != nullptr) {

		// init user data
		auto userData = new PhysBodyUserData;
		userData->destroyed = false;
		userData->gameObjectOwner = nullptr;
		this->m_body->GetUserData().pointer = reinterpret_cast<uintptr_t>(userData);

		this->m_bodyUserData = userData;

		// init fixtures
		for (const b2FixtureDef& fixtureDef : other->m_bodyFixtureDefs)
		{
			b2Fixture* fix = this->m_body->CreateFixture(&fixtureDef);

			b2Filter filter;
			filter.categoryBits = PlatformDataEngine::WORLD_STATIC;
			fix->SetFilterData(filter);
		}

		//if (PlatformDataEngineWrapper::getIsClient())
		//{
		//	this->m_body->SetType(b2BodyType::b2_kinematicBody);
		//}
	}
}

PhysicsBody::~PhysicsBody()
{
	if (!this->m_isDefinition && this->m_body != nullptr && m_bodyUserData != nullptr) {
		m_bodyUserData->destroyed = true;
	}
}


void PhysicsBody::init()
{
	Component::init();

	sf::Vector2f initPos = this->m_parent->getPosition();
	this->getBody()->SetTransform({
		initPos.x / Constants::PHYS_SCALE,
		initPos.y / Constants::PHYS_SCALE
		}, Utility::degToRad(this->m_parent->getRotation()));
	m_bodyUserData->gameObjectOwner = this->m_parent;
}

void PhysicsBody::update(const float& dt, const float& elapsedTime)
{
	this->m_parent->setPosition(
		this->m_body->GetTransform().p.x * Constants::PHYS_SCALE,
		this->m_body->GetTransform().p.y * Constants::PHYS_SCALE
	);

	if (this->m_doesRotate) {
		this->m_parent->setRotation(
			Utility::radToDeg(this->m_body->GetAngle())
		);
	}
}

void PhysicsBody::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void PhysicsBody::networkSerialize(PDEPacket& output)
{
	output
		<< this->m_body->GetPosition().x << this->m_body->GetPosition().y << this->m_body->GetAngle()
		<< this->m_body->GetLinearVelocity().x << this->m_body->GetLinearVelocity().y
		<< this->m_body->GetAngularVelocity();
}

void PhysicsBody::networkDeserialize(PDEPacket& input)
{
	float x = 0.0f, y = 0.0f, angle = 0.0f, vx = 0.0f, vy = 0.0f, va = 0.0f;
	input >> x >> y >> angle >> vx >> vy >> va;
	this->m_body->SetTransform({ x, y }, angle);
	this->m_body->SetLinearVelocity({vx, vy});
	this->m_body->SetAngularVelocity(va);

}

void PhysicsBody::loadDefinition(nlohmann::json object)
{
	if (object.at("type") == "dynamic")
	{
		this->m_bodyType = b2_dynamicBody;
	}
	else if (object.at("type") == "kinematic")
	{
		this->m_bodyType = b2_kinematicBody;
	}
	else
	{
		this->m_bodyType = b2_staticBody;
	}


	this->m_density = object.at("density");
	this->m_friction = object.at("friction");
	this->m_bouncy = object.at("bouncy");
	this->m_doesRotate = object.at("doesRotate");
	this->m_isContinuous = object.at("continuous");

	this->m_bodyDef.fixedRotation = !this->m_doesRotate;
	this->m_bodyDef.type = this->m_bodyType;
	this->m_bodyDef.position.Set(0, 0);
	this->m_bodyDef.bullet = this->m_isContinuous;
		
	for (const nlohmann::json& fixture : object.at("fixtures"))
	{
		sf::FloatRect bounds = sf::FloatRect(
			fixture.at("rect").at("x") / Constants::PHYS_SCALE,
			fixture.at("rect").at("y") / Constants::PHYS_SCALE,
			fixture.at("rect").at("width") / Constants::PHYS_SCALE,
			fixture.at("rect").at("height") / Constants::PHYS_SCALE
		);

		b2FixtureDef bodyFixtureDef;
		bodyFixtureDef.density = this->m_density;
		bodyFixtureDef.friction = this->m_friction;
		bodyFixtureDef.restitution = this->m_bouncy;

		if (fixture.at("shape") == "box") {
			b2PolygonShape* shape = new b2PolygonShape();
			std::array<b2Vec2, 4> pointArr = {
				b2Vec2(bounds.left, bounds.top),
				b2Vec2(bounds.left + bounds.width, bounds.top),
				b2Vec2(bounds.left + bounds.width, bounds.top + bounds.height),
				b2Vec2(bounds.left, bounds.top + bounds.height),
			};

			shape->Set(pointArr.data(), pointArr.size());
			bodyFixtureDef.shape = shape;
		}
		else if (fixture.at("shape") == "circle") {
			b2CircleShape* shape = new b2CircleShape();
			shape->m_p.Set(bounds.left, bounds.top + bounds.height);
			shape->m_radius = bounds.width / 2.0f;
			bodyFixtureDef.shape = shape;
		}

		this->m_bodyFixtureDefs.push_back(bodyFixtureDef);
	}

	this->m_bounds = sf::FloatRect(
		object.at("rect").at("x"),
		object.at("rect").at("y"),
		object.at("rect").at("width"),
		object.at("rect").at("height")
	);
}