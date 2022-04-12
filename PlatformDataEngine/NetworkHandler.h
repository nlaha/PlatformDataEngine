#pragma once
#include "GameWorld.h"

namespace PlatformDataEngine {
	class NetworkHandler
	{
	public:
		virtual void start() = 0;

		virtual void stop() = 0;

		virtual void process(GameWorld* world) = 0;
	};
}
