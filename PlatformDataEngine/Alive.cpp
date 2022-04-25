#include "Alive.h"
#include "PlatformDataEngineWrapper.h"

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
	if ((m_HP - damageAmount) <= 0.0f)
	{
		this->m_HP = 0.0f;
		if (!PlatformDataEngineWrapper::getIsClient()) {
			onDamage(this->m_HP);
			onDeath();
		}
	}
	else {
		// don't damage on client, sever will take care of that
		if (!PlatformDataEngineWrapper::getIsClient()) {
			this->m_HP -= damageAmount;
			onDamage(this->m_HP);
		}
	}
}
