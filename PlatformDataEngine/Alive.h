#pragma once

namespace PlatformDataEngine {

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
