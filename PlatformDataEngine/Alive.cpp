#include "Alive.h"

using namespace PlatformDataEngine;

/// <summary>
/// Constructor
/// </summary>
Alive::Alive()
{
	this->m_HP = 100.0;
}

/// <summary>
/// Deals damage and checks for death
/// </summary>
/// <param name="damageAmount">amount of damage to deal in % where an object at full health is 100%</param>
void Alive::damage(float damageAmount)
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
