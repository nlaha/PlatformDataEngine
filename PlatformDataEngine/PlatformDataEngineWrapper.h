#pragma once
#include <SFML/Graphics.hpp>

#include <filesystem>
#include <memory>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <thread>
#include <atomic>

#include "Utility.h"
#include "GameWorld.h"
#include "PlayerInputManager.h"
#include "PhysicsDebugDraw.h"
#include "Server.h"
#include "Client.h"

namespace PlatformDataEngine {

	enum PhysicsCategory {
		WORLD_STATIC = 0x0001,
		WORLD_DYNAMIC = 0x0002,
		CHARACTER = 0x0004,
		PROJECTILE = 0x0008,
		FRIEND_PROJECTILE = 0x0010,
		PARTICLE = 0x0012,
	};

	enum ApplicationMode {
		CLIENT,
		SERVER,
		DEDICATED
	};

	void renderingThread(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<GameWorld> world);

	/// <summary>
	/// Game Wrapper
	/// </summary>
	class PlatformDataEngineWrapper
	{
	public:
		PlatformDataEngineWrapper();
		~PlatformDataEngineWrapper();

		void run(ApplicationMode appMode);

		static inline float m_fps{ 0.0 };
		static inline sf::Vector2f m_windowZero{ 0.0, 0.0 };

		static inline std::shared_ptr<PlayerInputManager> getPlayerInputManager() {
			return PlatformDataEngineWrapper::mp_playerInputManager;
		}
		static inline std::shared_ptr<GameWorld> getWorld() {
			return PlatformDataEngineWrapper::mp_mainWorld;
		}
		static inline std::shared_ptr<sf::RenderWindow> getWindow() {
			return PlatformDataEngineWrapper::mp_renderWindow;
		}

		static inline bool getIsDebugPhysics() {
			return PlatformDataEngineWrapper::m_debugPhysics;
		}

		static inline NetworkHandler* getNetworkHandler() {
			return PlatformDataEngineWrapper::m_netHandler.get();
		}

		static inline bool getIsClient() {
			return PlatformDataEngineWrapper::m_isClient;
		}
	
	private:
		static std::shared_ptr<GameWorld> mp_mainWorld;
		static std::shared_ptr<PlayerInputManager> mp_playerInputManager;
		static std::shared_ptr<sf::RenderWindow> mp_renderWindow;
		static std::shared_ptr<NetworkHandler> m_netHandler;

		static bool m_pausedGame;
		static bool m_debugPhysics;
		static bool m_isClient;

		std::mutex mutex;
	};
}

