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

	void renderingThread(std::shared_ptr<sf::RenderWindow> window, GameWorld* world, std::atomic<bool>& threadStop);

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

		static inline std::string getPlayerName() {
			return PlatformDataEngineWrapper::m_playerName;
		}

		static inline void stopRenderThread() {
			PlatformDataEngineWrapper::m_renderThreadStop = true;
			m_renderThread.join();
		}

		static inline void startRenderThread() {
			PlatformDataEngineWrapper::m_renderThreadStop = false;
			mp_renderWindow->setActive(false);
			m_renderThread = std::thread(&renderingThread, mp_renderWindow, mp_mainWorld.get(), 
				std::ref(PlatformDataEngineWrapper::m_renderThreadStop));
		}

		static inline void quit() {
			mp_renderWindow->close();
		}

		static inline void loadServer() {

			spdlog::info("Loading server world please wait...");

			PlatformDataEngineWrapper::stopRenderThread();

			m_netHandler = std::make_shared<Server>();
			m_isClient = false;
			m_netHandler->start();

			PlatformDataEngineWrapper::mp_mainWorld = std::make_shared<GameWorld>();
			mp_mainWorld->initPhysics();

			PlatformDataEngineWrapper::mp_mainWorld->loadGameObjectDefinitions();


			PlatformDataEngineWrapper::mp_mainWorld->init("game/worlds/world.json", m_view, ApplicationMode::SERVER);

			/* Initialize Debug Draw */
			PlatformDataEngineWrapper::m_debugDraw = std::make_shared<PhysicsDebugDraw>(*mp_renderWindow);

			mp_mainWorld->getPhysWorld()->SetDebugDraw(PlatformDataEngineWrapper::m_debugDraw.get());
			PlatformDataEngineWrapper::m_debugDraw->SetFlags(b2Draw::e_shapeBit); //Only draw shapes

			PlatformDataEngineWrapper::startRenderThread();

			spdlog::info("Done loading server world!");
		}

		static inline void loadClient() {

			spdlog::info("Loading client world please wait...");

			PlatformDataEngineWrapper::stopRenderThread();

			m_netHandler = std::make_shared<Client>();
			m_isClient = true;
			m_netHandler->start();

			PlatformDataEngineWrapper::mp_mainWorld = std::make_shared<GameWorld>();
			mp_mainWorld->initPhysics();

			PlatformDataEngineWrapper::mp_mainWorld->loadGameObjectDefinitions();


			PlatformDataEngineWrapper::mp_mainWorld->initClient("game/worlds/world.json", m_view);

			/* Initialize Debug Draw */
			PlatformDataEngineWrapper::m_debugDraw = std::make_shared<PhysicsDebugDraw>(*mp_renderWindow);

			mp_mainWorld->getPhysWorld()->SetDebugDraw(PlatformDataEngineWrapper::m_debugDraw.get());
			PlatformDataEngineWrapper::m_debugDraw->SetFlags(b2Draw::e_shapeBit); //Only draw shapes

			PlatformDataEngineWrapper::startRenderThread();

			spdlog::info("Done loading client world!");
		}

		static std::string m_playerInput;
	
	private:
		static std::shared_ptr<GameWorld> mp_mainWorld;
		static std::shared_ptr<PlayerInputManager> mp_playerInputManager;
		static std::shared_ptr<sf::RenderWindow> mp_renderWindow;
		static std::shared_ptr<NetworkHandler> m_netHandler;
		static sf::View m_view;

		static bool m_pausedGame;
		static bool m_debugPhysics;
		static bool m_isClient;

		static std::string m_playerName;

		std::mutex mutex;
		static std::thread m_renderThread;
		static std::shared_ptr<PhysicsDebugDraw> m_debugDraw;
		static std::atomic<bool> m_renderThreadStop;
	};
}

