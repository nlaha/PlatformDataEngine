#pragma once
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine
{

	/// <summary>
	/// Class that defines an "alive" object, ie. an object
	/// that can take damage and die
	/// </summary>
	class Alive
	{
	public:
		Alive();

		void damage(float damageAmount);

		inline float getHealth() const { return m_HP; }

	protected:
		virtual void onDeath() = 0;
		virtual void onDamage(float currentHP) = 0;

		float m_HP;
	};
}
