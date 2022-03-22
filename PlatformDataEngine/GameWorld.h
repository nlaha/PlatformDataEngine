#pragma once
#include <memory>
#include <vector>
#include <map>
#include <box2d/box2d.h>
#include "TileMap.h"
#include "GameObject.h"
#include "Utility.h"
#include "CameraController.h"

namespace PlatformDataEngine {

	class GameWorld : public sf::Drawable
	{

		// NOTE: will contain a tile map, vector of game objects, etc

		// also contains physics world, has draw, update and physics update functions

	public:

		GameWorld();

		// game loops
		void init(std::string filePath, sf::View& view);
		void update(const float& dt, const float& elapsedTime);
		void physicsUpdate(const float& dt, const float& elapsedTime);
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		// helpers
		void registerGameObject(std::string name, std::shared_ptr<GameObject>);
		void registerGameObjectDefinition(std::string name, GameObject&);

		// getters
		inline std::shared_ptr<b2World> getPhysWorld() const { return this->mp_physicsWorld; };
		inline std::shared_ptr<TileMap> getTileMap() const { return this->mp_tileMap; };
		inline sf::View getView() const { return *this->mp_view; };
		inline std::shared_ptr<GameObject> getGameObject(std::string search) {
			return this->mp_gameObjects.find(search)->second;
		};

	private:
		std::shared_ptr<b2World> mp_physicsWorld;
		std::shared_ptr<TileMap> mp_tileMap;

		std::shared_ptr<sf::View> mp_view;
		CameraController m_cameraControl;

		std::map<std::string, std::shared_ptr<GameObject>> mp_gameObjects;
		std::map<std::string, GameObject> m_gameObjectDefinitions;
	};
}