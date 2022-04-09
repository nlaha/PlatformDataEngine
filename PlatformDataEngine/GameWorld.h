#pragma once
#include <memory>
#include <vector>
#include <chrono>
#include <map>
#include <box2d/box2d.h>
#include <spdlog/spdlog.h>
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
		void initPhysics();
		void update(const float& dt, const float& elapsedTime);
		void physicsUpdate(const float& dt, const float& elapsedTime);
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		// helpers
		void registerGameObject(std::string name, std::shared_ptr<GameObject>);
		void registerGameObjectDefinition(std::string name, std::shared_ptr<GameObject>);

		std::shared_ptr<GameObject> spawnGameObject(std::string type, sf::Vector2f position);

		// getters
		inline std::shared_ptr<b2World> getPhysWorld() const { return this->mp_physicsWorld; };
		inline std::shared_ptr<TileMap> getTileMap() const { return this->mp_tileMap; };
		inline const CameraController& getCameraController() const { return this->m_cameraControl; };
		inline sf::View getView() const { return *this->mp_view; };
		inline std::shared_ptr<GameObject> getGameObject(std::string search) {
			return this->mp_gameObjects.find(search)->second;
		};
		inline std::shared_ptr<GameObject> getPlayer() const { return this->mp_currentPlayer; };

		inline std::map<std::string, std::shared_ptr<GameObject>>& getGameObjectDefs() { return this->m_gameObjectDefinitions; };

		// setters
		inline void addPlayerSpawn(PlayerSpawn& spawn) {
			this->mp_playerSpawns.push_back(spawn);
		}

	private:
		std::shared_ptr<b2World> mp_physicsWorld;
		std::shared_ptr<TileMap> mp_tileMap;
		std::shared_ptr<GameObject> mp_currentPlayer;
		std::vector<PlayerSpawn> mp_playerSpawns;

		std::shared_ptr<sf::View> mp_view;

		CameraController m_cameraControl;

		std::map<std::string, std::shared_ptr<GameObject>> mp_gameObjects;

		std::map<std::string, std::shared_ptr<GameObject>> m_gameObjectDefinitions;
	};
}