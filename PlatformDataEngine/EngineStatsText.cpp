#include "EngineStatsText.h"
#include "PlatformDataEngineWrapper.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

void EngineStatsText::update(const float& dt, const float& elapsedTime)
{
	this->setText(fmt::format("FPS: {0:.0f} | DT: {1:.4f}", PlatformDataEngineWrapper::m_fps, dt));
}