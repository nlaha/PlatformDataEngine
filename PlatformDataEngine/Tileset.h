#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <spdlog/spdlog.h>

#include "TileTexture.h"
#include "TileSprite.h"

namespace PlatformDataEngine {

    class TileSprite;

    /// <summary>
    /// Defines a tileset used in a tmx map. This is what actually gets drawn when
    /// target.draw(tileMap) is called, it will loop through all the tilesets and draw them
    /// each in a separate draw call
    /// </summary>
    class Tileset : public sf::Drawable
    {
    public:
        Tileset(const std::string& imagePath, int tileSize, const std::string& shaderPath = "");

        const TileTexture& getTileTexture(int tileIdx) const;
        void registerSprite(TileSprite* sprite);
        void deregisterSprite(TileSprite* sprite);
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
        void update(const float& dt, const float& elapsedTime);

        inline int getTileSize() const { return this->m_tileSize; };
        inline std::shared_ptr<sf::Shader> getShader() const { return this->m_shader; };

    private:
        bool loadTileset(const std::string& imagePath, const std::string& shaderPath);

        std::vector<TileTexture> m_tileTextures;
        std::vector<TileSprite*> m_tileSprites;
        std::unique_ptr<sf::Texture> m_texture;
        std::shared_ptr<sf::Shader> m_shader;
        int m_tileSize;
    };
}

