#include "Alive.h"

PlatformDataEngine::Alive::Alive()
{
	this->m_HP = 100.0;
}

void PlatformDataEngine::Alive::damage(float damageAmount)
{
	this->m_HP -= damageAmount;
	onDamage(this->m_HP);

	if (this->m_HP <= 0) {
		onDeath();
	}
}
