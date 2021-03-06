#include "TextDrawable.h"

using namespace PlatformDataEngine;

/// <summary>
/// Default constructor
/// </summary>
TextDrawable::TextDrawable(std::string fontFilename)
{
	this->m_font.loadFromFile(fontFilename);
	this->m_fontBold = false;
	this->m_fontSize = 70;
	this->m_quads = sf::VertexArray(sf::Quads);
	this->m_renderSize = 1.0f;
	this->m_text = "";
	this->m_isCentered = true;
	this->m_shader = std::make_shared<sf::Shader>();
	this->m_shader->loadFromFile("shaders/text.vert", "shaders/text.frag");
	this->m_shader->setUniform("textColor", sf::Glsl::Vec4(sf::Color::White));
}

/// <summary>
/// Constructor
/// </summary>
/// <param name="fontSize">the font size</param>
/// <param name="bold">if true, bold font</param>
/// <param name="renderSize">the render size, sort of like scale, independant from font size</param>
/// <param name="text">the text to draw</param>
TextDrawable::TextDrawable(int fontSize, bool bold, float renderSize, const std::string& text)
{
	this->m_font.loadFromFile("assets/smallest_pixel-7.ttf");
	this->m_fontBold = bold;
	this->m_fontSize = fontSize;
	this->m_quads = sf::VertexArray(sf::Quads);
	this->m_renderSize = renderSize;
	this->m_text = text;
	this->m_shader = std::make_shared<sf::Shader>();
	this->m_shader->loadFromFile("shaders/text.vert", "shaders/text.frag");
	this->m_shader->setUniform("textColor", sf::Glsl::Vec4(sf::Color::White));
}

/// <summary>
/// Sets the new text in the text drawable
/// </summary>
/// <param name="text">text to set</param>
void TextDrawable::setText(const std::string& text)
{
	if (text != this->m_text) {
		this->m_quads.clear();

		this->m_text = text;

		auto lambdaAddQuad = [&](sf::Glyph glyph, float& leftOffset, float renderSize) {
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

			m_quads.append(quad[0]);
			m_quads.append(quad[1]);
			m_quads.append(quad[2]);
			m_quads.append(quad[3]);
		};

		if (this->m_text.size() > 0)
		{
			int accumulatedAdv = 0;
			for (size_t i = 0; i < this->m_text.size(); i++)
			{
				sf::Glyph glyph = this->m_font.getGlyph(this->m_text[i], this->m_fontSize, this->m_fontBold);
				if (i != 0) {
					sf::Glyph prevGlyph = this->m_font.getGlyph(this->m_text[(i - 1) < 0 || (i - 1) > this->m_text.size() ? 0 : i - 1], this->m_fontSize, this->m_fontBold);
					accumulatedAdv += prevGlyph.advance;
				}
				float leftOffset =
					((float)accumulatedAdv * this->m_renderSize) +
					m_font.getKerning(i - 1, i, this->m_fontSize);

				lambdaAddQuad(glyph, leftOffset, this->m_renderSize);
			}
		}		
	}
}

/// <summary>
/// Draws the text, never call directly
/// always use target.draw(textDrawable);
/// </summary>
/// <param name="target"></param>
/// <param name="states"></param>
void TextDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.shader = this->m_shader.get();
	states.transform *= getTransform();
	if (this->m_isCentered) {
		states.transform.translate({ -(m_quads.getBounds().width / 2.0f), 0.0f });
	}

	sf::Texture texture = this->m_font.getTexture(this->m_fontSize);
	texture.setSmooth(false);

	states.texture = &texture;
	target.draw(this->m_quads, states);
}
