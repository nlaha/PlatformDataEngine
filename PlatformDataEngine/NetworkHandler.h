#pragma once
#include "GameWorld.h"

namespace PlatformDataEngine {
	class NetworkHandler
	{
	public:
		virtual void start() = 0;

		virtual void stop() = 0;

		/// <summary>
		/// Process is responsible for sending packets, limited in rate
		/// </summary>
		/// <param name="world"></param>
		virtual void process(GameWorld* world) = 0;

		/// <summary>
		/// Recieve checks for new packets, not limited in rate
		/// </summary>
		/// <param name="world"></param>
		virtual void recieve(GameWorld* world) = 0;
	};
}
