#pragma once
#include <memory>
#include <vector>
#include <chrono>
#include <map>
#include <thread>
#include <box2d/box2d.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <string>
#include <SFML/Audio.hpp>
#include "TileMap.h"
#include "GameObject.h"
#include "Utility.h"
#include "PhysicsCallbacks.h"
#include "CameraController.h"

namespace PlatformDataEngine {

	enum ApplicationMode;

	/// <summary>
	/// Represents a game world with it's own tile map and gameObjects
	/// </summary>
	class GameWorld : public sf::Drawable
	{

		// NOTE: will contain a tile map, vector of game objects, etc

		// also contains physics world, has draw, update and physics update functions

	public:

		struct PlayerSpawn {
			sf::Vector2f position;
		};

		GameWorld();

		// game functions and loops
		void init(const std::string& filePath, sf::View& view, ApplicationMode appMode);
		void initClient(const std::string& filePath, sf::View& view);
		void initPhysics();
		void update(const float& dt, const float& elapsedTime);
		void physicsUpdate(const float& dt, const float& elapsedTime);
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void networkThread();

		// helpers
		void registerGameObject(const std::string& name, std::shared_ptr<GameObject> gameObject);
		void registerGameObjectDefinition(const std::string& name, std::shared_ptr<GameObject> gameObjectDef);
		void loadGameObjectDefinitions();

		std::shared_ptr<GameObject> spawnGameObject(const std::string& type, sf::Vector2f position, std::string id = "", bool noReplication = false, float rotation = 0, sf::Vector2f origin = { 0.0, 0.0 }, bool alreadyReplicated = false);

		// getters
		inline std::shared_ptr<sf::Thread> getNetThread() const { return this->mp_networkingThread; };
		inline std::map<std::string, std::shared_ptr<GameObject>>& getGameObjects() { return this->mp_gameObjects; };
		inline std::shared_ptr<b2World> getPhysWorld() const { return this->mp_physicsWorld; };
		inline std::shared_ptr<TileMap> getTileMap() const { return this->mp_tileMap; };
		inline CameraController& getCameraController() { return this->m_cameraControl; };
		inline sf::View getView() const { return *this->mp_view; };
		inline std::shared_ptr<GameObject> getGameObject(const std::string& search) {
			if (mp_gameObjects.count(search) > 0) {
				return this->mp_gameObjects.at(search);
			}
			else {
				return nullptr;
			}
		};
		inline GameObject* getPlayer() const { return this->mp_currentPlayer; };
		inline GameObject* getPlayer(std::shared_ptr<Connection> conn) const {
			GameObject* out = nullptr;
			try {
				out = this->m_players.at(conn);
			}
			catch (std::out_of_range)
			{
				return nullptr;
			}
			return out;
		};

		inline std::map<std::string, std::shared_ptr<GameObject>>& getGameObjectDefs() { return this->m_gameObjectDefinitions; };


		// setters
		std::string spawnPlayer(std::shared_ptr<Connection> conn);

		inline void setPlayer(GameObject* player) { this->mp_currentPlayer = player; }

		inline void addPlayerSpawn(PlayerSpawn& spawn) {
			this->mp_playerSpawns.push_back(spawn);
		}

		inline void setInGame(bool inGame) {
			this->m_inGame = inGame;
		}

		inline bool getInGame() { return this->m_inGame; };

		inline const std::vector<GameObject*> getNetworkObjects() const { return this->mp_networkObjects; };
		std::map<std::shared_ptr<Connection>, GameObject*> getPlayers() const { return this->m_players; };

	private:
		std::shared_ptr<sf::Thread> mp_networkingThread;

		std::shared_ptr<b2World> mp_physicsWorld;
		std::shared_ptr<TileMap> mp_tileMap;
		GameObject* mp_currentPlayer;
		std::vector<PlayerSpawn> mp_playerSpawns;

		std::shared_ptr<sf::View> mp_view;

		sf::Clock m_packetClock;
		sf::Clock m_garbageClock;

		CameraController m_cameraControl;

		std::shared_ptr<ContactFilter> m_physFilter;

		std::vector<GameObject*> mp_networkObjects;
		std::map<std::string, std::shared_ptr<GameObject>> mp_gameObjects;
		std::map<std::shared_ptr<Connection>, GameObject*> m_players;
		nlohmann::json m_playerDef;

		bool m_inGame;
		int m_spawnIdx;

		std::map<std::string, std::shared_ptr<GameObject>> m_gameObjectDefinitions;

		std::shared_ptr<GameObject> spawnDefinedGameObject(nlohmann::json gameObject, std::string name = "");
	
		TextDrawable m_youDiedText;
		TextDrawable m_respawnTimerText;

		void garbageCollect();
	};
}