#include "PhysicsDebugDraw.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

PhysicsDebugDraw::PhysicsDebugDraw(sf::RenderWindow& window) : m_window(&window) {}

/*
Taken from:
https://github.com/MrPlow442/Box2D-SFML2-Debug-Draw
*/

void PhysicsDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	sf::ConvexShape polygon(vertexCount);
	sf::Vector2f center;
	for (int i = 0; i < vertexCount; i++)
	{
		//polygon.setPoint(i, SFMLDraw::B2VecToSFVec(vertices[i]));
		sf::Vector2f transformedVec = PhysicsDebugDraw::B2VecToSFVec(vertices[i]);
		polygon.setPoint(i, sf::Vector2f(transformedVec.x, transformedVec.y));
	}																								  
	polygon.setOutlineThickness(-1.f);
	polygon.setFillColor(sf::Color::Transparent);
	polygon.setOutlineColor(PhysicsDebugDraw::GLColorToSFML(color));

	m_window->draw(polygon);
}
void PhysicsDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	sf::ConvexShape polygon(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		//polygon.setPoint(i, SFMLDraw::B2VecToSFVec(vertices[i]));
		sf::Vector2f transformedVec = PhysicsDebugDraw::B2VecToSFVec(vertices[i]);
		polygon.setPoint(i, sf::Vector2f(transformedVec.x, transformedVec.y));
	}																								   
	polygon.setOutlineThickness(-1.f);
	polygon.setFillColor(PhysicsDebugDraw::GLColorToSFML(color, 60));
	polygon.setOutlineColor(PhysicsDebugDraw::GLColorToSFML(color));

	m_window->draw(polygon);
}
void PhysicsDebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	sf::CircleShape circle(radius * Constants::PHYS_SCALE);
	circle.setOrigin(radius * Constants::PHYS_SCALE, radius * Constants::PHYS_SCALE);
	circle.setPosition(PhysicsDebugDraw::B2VecToSFVec(center));
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(-1.f);
	circle.setOutlineColor(PhysicsDebugDraw::GLColorToSFML(color));

	m_window->draw(circle);
}
void PhysicsDebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	sf::CircleShape circle(radius * Constants::PHYS_SCALE);
	circle.setOrigin(radius * Constants::PHYS_SCALE, radius * Constants::PHYS_SCALE);
	circle.setPosition(PhysicsDebugDraw::B2VecToSFVec(center));
	circle.setFillColor(PhysicsDebugDraw::GLColorToSFML(color, 60));
	circle.setOutlineThickness(1.f);
	circle.setOutlineColor(PhysicsDebugDraw::GLColorToSFML(color));

	b2Vec2 endPoint = center + radius * axis;
	sf::Vertex line[2] =
	{
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(center), PhysicsDebugDraw::GLColorToSFML(color)),
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(endPoint), PhysicsDebugDraw::GLColorToSFML(color)),
	};

	m_window->draw(circle);
	m_window->draw(line, 2, sf::Lines);
}
void PhysicsDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	sf::Vertex line[] =
	{
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(p1), PhysicsDebugDraw::GLColorToSFML(color)),
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(p2), PhysicsDebugDraw::GLColorToSFML(color))
	};

	m_window->draw(line, 2, sf::Lines);
}
void PhysicsDebugDraw::DrawTransform(const b2Transform& xf)
{
	float lineLength = 0.4;

	/*b2Vec2 xAxis(b2Vec2(xf.p.x + (lineLength * xf.q.c), xf.p.y + (lineLength * xf.q.s)));*/
	b2Vec2 xAxis = xf.p + lineLength * xf.q.GetXAxis();
	sf::Vertex redLine[] =
	{
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(xf.p), sf::Color::Red),
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(xAxis), sf::Color::Red)
	};

	// You might notice that the ordinate(Y axis) points downward unlike the one in Box2D testbed
	// That's because the ordinate in SFML coordinate system points downward while the OpenGL(testbed) points upward
	/*b2Vec2 yAxis(b2Vec2(xf.p.x + (lineLength * -xf.q.s), xf.p.y + (lineLength * xf.q.c)));*/
	b2Vec2 yAxis = xf.p + lineLength * xf.q.GetYAxis();
	sf::Vertex greenLine[] =
	{
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(xf.p), sf::Color::Green),
		sf::Vertex(PhysicsDebugDraw::B2VecToSFVec(yAxis), sf::Color::Green)
	};

	m_window->draw(redLine, 2, sf::Lines);
	m_window->draw(greenLine, 2, sf::Lines);
}

void PhysicsDebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
	sf::CircleShape circle(size * Constants::PHYS_SCALE);
	circle.setOrigin(size * Constants::PHYS_SCALE, size * Constants::PHYS_SCALE);
	circle.setPosition(PhysicsDebugDraw::B2VecToSFVec(p));
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(-1.f);
	circle.setOutlineColor(PhysicsDebugDraw::GLColorToSFML(color));

	m_window->draw(circle);
}
