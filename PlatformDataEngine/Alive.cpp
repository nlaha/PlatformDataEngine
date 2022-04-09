#include "Alive.h"

/// <summary>
/// Constructor
/// </summary>
PlatformDataEngine::Alive::Alive()
{
	this->m_HP = 100.0;
}

/// <summary>
/// Deals damage and checks for death
/// </summary>
/// <param name="damageAmount">amount of damage to deal in % where an object at full health is 100%</param>
void PlatformDataEngine::Alive::damage(float damageAmount)
{
	if (this->m_HP - damageAmount <= 0) {
		this->m_HP = 0;
		onDeath();
	}
	else {
		this->m_HP -= damageAmount;
		onDamage(this->m_HP);
	}
}

inline void PlatformDataEngine::Alive::onDeath()
{
	// todo: get "death handler" component if it exists and call onDeath()
}

inline void PlatformDataEngine::Alive::onDamage(float currentHP)
{
	// todo: get "damage handler" component if it exists and call onDamage()
}
