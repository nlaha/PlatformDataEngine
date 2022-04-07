#pragma once

namespace PlatformDataEngine {

	/// <summary>
	/// Class that defines an "alive" object, ie. an object 
	/// that can take damage and die
	/// </summary>
	class Alive
	{
	public:
		Alive();

		void damage(float damageAmount);

	protected:
		virtual inline void onDeath() {};
		virtual inline void onDamage(float currentHP) {};

	private:
		float m_HP;
	};
}
