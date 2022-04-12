#include "TextRenderer.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

TextRenderer::TextRenderer()
{
	this->m_font = sf::Font();
	this->m_font.loadFromFile("assets/VT323-Regular.ttf");
	this->m_fontBold = false;
	this->m_fontSize = 70;
	this->m_renderSize = 0.1f;
}

void TextRenderer::init()
{
	Component::init();
}

void TextRenderer::update(const float &dt, const float &elapsedTime)
{
}

void TextRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	sf::Texture texture = this->m_font.getTexture(this->m_fontSize);
	texture.setSmooth(false);

	// create a vertex array for the text
	sf::VertexArray quads(sf::Quads);

	auto lambdaAddQuad = [](sf::Glyph glyph, float& leftOffset, float renderSize, sf::VertexArray& quads) {
		sf::Vertex quad[4];

		glyph.bounds.left *= renderSize;
		glyph.bounds.left += leftOffset;
		glyph.bounds.top *= renderSize;

		glyph.bounds.width *= renderSize;
		glyph.bounds.height *= renderSize;

		// define its 4 corners
		quad[0].position = sf::Vector2f(glyph.bounds.left, glyph.bounds.top);
		quad[1].position = sf::Vector2f(glyph.bounds.left + glyph.bounds.width, glyph.bounds.top);
		quad[2].position = sf::Vector2f(glyph.bounds.left + glyph.bounds.width, glyph.bounds.top + glyph.bounds.height);
		quad[3].position = sf::Vector2f(glyph.bounds.left, glyph.bounds.top + glyph.bounds.height);

		// define its 4 texture coordinates
		quad[0].texCoords = sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top);
		quad[1].texCoords = sf::Vector2f(glyph.textureRect.left + glyph.textureRect.width, glyph.textureRect.top);
		quad[2].texCoords = sf::Vector2f(glyph.textureRect.left + glyph.textureRect.width, glyph.textureRect.top + glyph.textureRect.height);
		quad[3].texCoords = sf::Vector2f(glyph.textureRect.left, glyph.textureRect.top + glyph.textureRect.height);

		quads.append(quad[0]);
		quads.append(quad[1]);
		quads.append(quad[2]);
		quads.append(quad[3]);
	};

	if (this->m_text.size() > 0)
	{
		int accumulatedAdv = 0;
		for (size_t i = 0; i < this->m_text.size(); i++)
		{
			sf::Glyph prevGlyph = this->m_font.getGlyph(this->m_text[(i - 1) < 0 || (i - 1) > this->m_text.size() ? 0 : i - 1], this->m_fontSize, this->m_fontBold);
			sf::Glyph glyph = this->m_font.getGlyph(this->m_text[i], this->m_fontSize, this->m_fontBold);
			accumulatedAdv += prevGlyph.advance;

			float leftOffset =
				((float)accumulatedAdv * this->m_renderSize) +
				m_font.getKerning(i - 1, i, this->m_fontSize);

			lambdaAddQuad(glyph, leftOffset, this->m_renderSize, quads);
		}
	}

	states.texture = &texture;
	target.draw(quads, states);
}

void TextRenderer::copy(std::shared_ptr<Component> otherCompPtr)
{
	std::shared_ptr<TextRenderer> other = std::dynamic_pointer_cast<TextRenderer>(otherCompPtr);

	*this = *other;
}

void TextRenderer::loadDefinition(nlohmann::json object)
{
	this->m_font = sf::Font();
	this->m_font.loadFromFile(object.at("fontPath"));
	this->m_fontBold = object.at("fontBold");
	this->m_fontSize = object.at("fontSize");
	this->m_text = object.at("text");
	this->m_renderSize = object.at("renderSize");
}
