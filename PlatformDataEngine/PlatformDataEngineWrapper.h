#pragma once
#include <SFML/Graphics.hpp>

#include <filesystem>
#include <memory>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "Utility.h"
#include "GameWorld.h"

namespace PlatformDataEngine {

	/// <summary>
	/// Game Wrapper
	/// </summary>
	class PlatformDataEngineWrapper
	{
	public:
		PlatformDataEngineWrapper();
		~PlatformDataEngineWrapper();

		static std::shared_ptr<GameWorld> mp_mainWorld;

		void run();

		static inline float m_fps{ 0.0 };
		static inline sf::Vector2f m_windowZero{ 0.0, 0.0 };

		static inline std::shared_ptr<GameWorld> getWorld() {
			return PlatformDataEngineWrapper::mp_mainWorld;
		}

	private:

	};
}

