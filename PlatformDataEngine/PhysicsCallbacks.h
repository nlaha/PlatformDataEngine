#pragma once
#include <box2d/box2d.h>
#include <spdlog/spdlog.h>

namespace PlatformDataEngine {

    class GameObject;

    /// <summary>
    /// Data stored in physics bodies that
    /// are attached to a game object
    /// </summary>
    struct PhysBodyUserData
    {
        bool destroyed;
        GameObject* gameObjectOwner;
    };

    /// <summary>
    /// Contact filter, overrides default to prevent rockets
    /// from colliding with player
    /// </summary>
    class ContactFilter : public b2ContactFilter
    {
    public:
        bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);
    };

    /// <summary>
    /// Not used
    /// </summary>
    class BoxQueryCallback : public b2QueryCallback
    {
    public:
        bool ReportFixture(b2Fixture* fixture)
        {
            b2Body* body = fixture->GetBody();

            // Return true to continue the query.
            return true;
        }
    };

	/// <summary>
	/// Used when Box2d does a raycast
	/// </summary>
	class RaycastCallback : public b2RayCastCallback
	{
        public:
            RaycastCallback()
            {
                m_fixture = NULL;
            }

            inline float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                const b2Vec2& normal, float fraction)
            {
                m_fixture = fixture;
                m_point = point;
                m_normal = normal;
                m_fraction = fraction;
                return fraction;
            };

            inline bool isBody(b2Body* pBody) {
                return this->m_fixture->GetBody() == pBody;
            };

            b2Fixture* m_fixture;
            b2Vec2 m_point;
            b2Vec2 m_normal;
            float m_fraction;
	};
}

