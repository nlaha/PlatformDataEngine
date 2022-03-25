#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <execution>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>
#include <spdlog/spdlog.h>

#include "Tileset.h"
#include "TileSprite.h"

namespace PlatformDataEngine {

    class TileMap : public sf::Drawable
    {
    public:
        TileMap(const std::string& tmxPath);

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
        void update(const float& dt, const float& elapsedTime);

        struct TilesetPair
        {
            tmx::Tileset tmxTileset;
            std::shared_ptr<Tileset> tileset;
        };

        std::vector<TilesetPair> getTilesets() const { return this->m_tilesets; };

    private:
        std::vector<TilesetPair> m_tilesets;
        std::vector<std::unique_ptr<TileSprite>> m_tileSprites;

    };
}
