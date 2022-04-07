#include "TextRenderer.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

void TextRenderer::init()
{
	Component::init();

}

void TextRenderer::update(const float& dt, const float& elapsedTime)
{
}

void TextRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// this is all terrible please someone fix I beg of you...
	// ideally we'd construct a sequence of quads whenever the text
	// is changed instead of this abomination that creates sprites
	// for each individual character in the string

	sf::Texture texture = this->m_font.getTexture(this->m_fontSize);
	texture.setSmooth(false);

	if (this->m_text.size() > 0) {
		int accumulatedAdv = 0;
		for (size_t i = 0; i < this->m_text.size(); i++)
		{
			sf::Glyph prevGlyph = this->m_font.getGlyph(this->m_text[(i - 1) < 0 || (i - 1) > this->m_text.size() ? 0 : i - 1], this->m_fontSize, this->m_fontBold);
			sf::Glyph glyph = this->m_font.getGlyph(this->m_text[i], this->m_fontSize, this->m_fontBold);
			accumulatedAdv += prevGlyph.advance;

			sf::Sprite charSprite(texture, glyph.textureRect);
			charSprite.setPosition({ 
				((float)accumulatedAdv * this->m_renderSize) +
				m_font.getKerning(i - 1, i, this->m_fontSize) +
				(glyph.bounds.left * this->m_renderSize), 
				glyph.bounds.top * this->m_renderSize 
			});
			charSprite.setScale({ this->m_renderSize, this->m_renderSize });
			target.draw(charSprite, states);
		}
	}
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
