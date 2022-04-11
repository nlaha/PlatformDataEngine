#include "TileMap.h"
#include "PlatformDataEngineWrapper.h"

#include "Globals.h"

using namespace PlatformDataEngine;

std::mutex mutex;

/// <summary>
/// Constructor
/// </summary>
TileMap::TileMap(const std::string &tmxPath)
{
    tmx::Map map;
    map.load(tmxPath);

    // Get the tilesets
    for (tmx::Tileset tileset : map.getTilesets())
    {
        std::string shaderFile = "";
        if (tileset.getName() == "Plants")
        {
            shaderFile = "shaders/foliage";
        }

        std::shared_ptr<Tileset> ts = std::make_shared<Tileset>(tileset.getImagePath(), tileset.getTileSize().x, shaderFile);

        this->m_tilesets.push_back({tileset, ts});
    }

    // Get layers
    for (auto &layerRef : map.getLayers())
    {
        spdlog::info("Found layer with type {}", layerRef->getName());

        if (layerRef->getType() == tmx::Layer::Type::Tile)
        {
            if (map.getOrientation() == tmx::Orientation::Orthogonal)
            {
                const auto tileSize = map.getTileSize().x;
                const auto &layer = layerRef->getLayerAs<tmx::TileLayer>();

                bool hasPhysics = false;

                std::vector<tmx::Property> layerProps = layer.getProperties();
                for (tmx::Property prop : layerProps)
                {
                    if (prop.getName() == "hasPhysics")
                    {
                        hasPhysics = prop.getBoolValue();
                    }
                }

                // loop through chunks
                // use parallel for_each
                std::mutex mutex;
                std::for_each(std::execution::par, layer.getChunks().begin(), layer.getChunks().end(), [&](tmx::TileLayer::Chunk chunk)
                {
                    sf::Vector2f chunkOrigin(chunk.position.x * 8, chunk.position.y * 8);
                    for (size_t x = 0; x < chunk.size.x; x++)
                    {
                        for (size_t y = 0; y < chunk.size.y; y++)
                        {
                            sf::Vector2f tilePos = { chunkOrigin.x + (float)x * tileSize, chunkOrigin.y + (float)y * tileSize };
                            int tileId = chunk.tiles[x + y * chunk.size.x].ID;

                            // find the tileset for this tile
                            for (const auto& tilesetPair : this->m_tilesets)
                            {
                                int tSize = tilesetPair.tmxTileset.getTileSize().x;

                                if (tileId >= tilesetPair.tmxTileset.getFirstGID() && tileId <= tilesetPair.tmxTileset.getLastGID())
                                {
                                    tmx::ObjectGroup collision = tilesetPair.tmxTileset.getTile(tileId)->objectGroup;

                                    b2PolygonShape collisionShape;

                                    if (collision.getObjects().size() > 0) {
                                        tmx::Object obj = collision.getObjects().front();
                                        std::vector<b2Vec2> pointArr;
                                        tmx::Vector2f oPos = obj.getPosition();
                                        for (tmx::Vector2f point : obj.getPoints()) {
                                            point.x *= 1.0001;
                                            point.y *= 1.0001;
                                            pointArr.push_back({ 
                                                (point.x + oPos.x) / Constants::PHYS_SCALE, 
                                                (point.y + oPos.y) / Constants::PHYS_SCALE });
                                        }

                                        collisionShape.Set(pointArr.data(), pointArr.size());
                                    }
                                    else {
                                        hasPhysics = false;
                                    }


                                    std::lock_guard<std::mutex> guard(mutex);
                                    // create a tile sprite
                                    std::unique_ptr<TileSprite> tileSprite = std::make_unique<TileSprite>(
                                        tilesetPair.tileset,
                                        collisionShape,
                                        tileId - tilesetPair.tmxTileset.getFirstGID(),
                                        hasPhysics,
                                        tilePos);

                                    // add the tile sprite to the map
                                    this->m_tileSprites.push_back(std::move(tileSprite));
                                }
                            }
                        }
                    }
                });
            }
            else
            {
                spdlog::info("Not a valid orthogonal layer, nothing will be drawn.");
            }
        }
        else if (layerRef->getType() == tmx::Layer::Type::Object)
        {
            const auto& objectLayer = layerRef->getLayerAs<tmx::ObjectGroup>();
            const auto& objects = objectLayer.getObjects();
            for (const auto& object : objects)
            {
                if (object.getType() == "PlayerSpawn")
                {
                    GameWorld::PlayerSpawn spawn = {
                        sf::Vector2f(object.getPosition().x, object.getPosition().y - 16)
                    };
                    PlatformDataEngineWrapper::getWorld()->addPlayerSpawn(spawn);
                }
                else if (object.getType() == "Prop")
                {
                    int tileId = object.getTileID();

                    // find the tileset for this tile
                    for (const auto& tilesetPair : this->m_tilesets)
                    {
                        int tSize = tilesetPair.tmxTileset.getTileSize().x;

                        if (tileId >= tilesetPair.tmxTileset.getFirstGID() && tileId <= tilesetPair.tmxTileset.getLastGID())
                        {
                            tmx::Vector2u pos = tilesetPair.tmxTileset.getTile(tileId)->imagePosition;
                            tmx::ObjectGroup collision = tilesetPair.tmxTileset.getTile(tileId)->objectGroup;

                            std::shared_ptr<GameObject> prop = PlatformDataEngineWrapper::getWorld()->spawnGameObject("Prop",
                                sf::Vector2f(object.getPosition().x, object.getPosition().y - tSize));

                            prop->findComponentOfType<SpriteRenderer>()->setRect(sf::IntRect(
                                sf::Vector2i(pos.x, pos.y),
                                sf::Vector2i(tSize, tSize)
                            ));

                            std::shared_ptr<PhysicsBody> pBody = prop->findComponentOfType<PhysicsBody>();
                            if (collision.getObjects().size() > 0) {
                                tmx::Object obj = collision.getObjects().front();
                                std::vector<b2Vec2> pointArr;
                                tmx::Vector2f oPos = obj.getPosition();
                                for (tmx::Vector2f point : obj.getPoints()) {
                                    point.x *= 1.0001;
                                    point.y *= 1.0001;
                                    pointArr.push_back({ 
                                        (point.x + oPos.x) / Constants::PHYS_SCALE, 
                                        (point.y + oPos.y) / Constants::PHYS_SCALE });
                                }

                                b2PolygonShape poly;
                                poly.Set(pointArr.data(), pointArr.size());

                                b2FixtureDef bodyFixtureDef;
                                bodyFixtureDef.density = pBody->getDensity();
                                bodyFixtureDef.friction = pBody->getFriction();
                                bodyFixtureDef.restitution = pBody->getBouncy();
                                bodyFixtureDef.shape = &poly;

                                pBody->getBody()->CreateFixture(&bodyFixtureDef);
                            }
                        }
                    }
                }
            }
        }
    }
}

void TileMap::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    for (const auto &tilesetPair : this->m_tilesets)
    {
        target.draw(*tilesetPair.tileset, states);
    }
}

void TileMap::update(const float& dt, const float& elapsedTime)
{
    for (const auto& tilesetPair : this->m_tilesets)
    {
        tilesetPair.tileset->update(dt, elapsedTime);
    }
}
