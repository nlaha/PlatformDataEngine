#pragma once
#include <memory>
#include <vector>
#include <chrono>
#include <map>
#include <thread>
#include <box2d/box2d.h>
#include <spdlog/spdlog.h>
#include <string>
#include "TileMap.h"
#include "GameObject.h"
#include "Utility.h"
#include "PhysicsCallbacks.h"
#include "CameraController.h"

namespace PlatformDataEngine {

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
		void init(std::string filePath, sf::View& view);
		void initClient(std::string filePath, sf::View& view);
		void initPhysics();
		void update(const float& dt, const float& elapsedTime);
		void physicsUpdate(const float& dt, const float& elapsedTime);
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		// helpers
		void registerGameObject(std::string name, std::shared_ptr<GameObject> gameObject);
		void registerGameObjectDefinition(std::string name, std::shared_ptr<GameObject> gameObjectDef);

		std::shared_ptr<GameObject> spawnGameObject(std::string type, sf::Vector2f position, std::string name = "", bool noReplication = false);

		// getters
		inline std::map<std::string, std::shared_ptr<GameObject>>& getGameObjects() { return this->mp_gameObjects; };
		inline std::shared_ptr<b2World> getPhysWorld() const { return this->mp_physicsWorld; };
		inline std::shared_ptr<TileMap> getTileMap() const { return this->mp_tileMap; };
		inline const CameraController& getCameraController() const { return this->m_cameraControl; };
		inline sf::View getView() const { return *this->mp_view; };
		inline std::shared_ptr<GameObject> getGameObject(std::string search) {
			if (mp_gameObjects.find(search) != mp_gameObjects.end()) {
				return this->mp_gameObjects.find(search)->second;
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

		inline std::vector<std::string> getNetToDestroy() const {
			return this->mp_netToDestroy;
		}

		inline void clearNetDestroy() {
			this->mp_netToDestroy.clear();
		}

		inline void addNetToDestroy(std::string name) {
			if (name != "") {
				this->mp_netToDestroy.push_back(name);
			}
		}


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

		std::map<std::shared_ptr<Connection>, GameObject*> getPlayers() const { return this->m_players; };

	private:
		std::shared_ptr<b2World> mp_physicsWorld;
		std::shared_ptr<TileMap> mp_tileMap;
		GameObject* mp_currentPlayer;
		std::vector<PlayerSpawn> mp_playerSpawns;

		std::shared_ptr<sf::View> mp_view;

		sf::Clock m_packetClock;
		sf::Clock m_garbageClock;

		CameraController m_cameraControl;

		std::shared_ptr<ContactFilter> m_physFilter;

		std::vector<std::string> mp_netToDestroy;
		std::map<std::string, std::shared_ptr<GameObject>> mp_gameObjects;
		std::map<std::shared_ptr<Connection>, GameObject*> m_players;
		nlohmann::json m_playerDef;

		bool m_inGame;
		int m_spawnIdx;

		std::map<std::string, std::shared_ptr<GameObject>> m_gameObjectDefinitions;

		std::shared_ptr<GameObject> spawnDefinedGameObject(nlohmann::json gameObject, std::string name = "");
	
		void garbageCollect();
	};
}