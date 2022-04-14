#include "TextRenderer.h"
#include "GameObject.h"

using namespace PlatformDataEngine;

TextRenderer::TextRenderer()
{
	this->m_font = sf::Font();
	this->m_font.loadFromFile("assets/VT323-Regular.ttf");
	this->m_textDrawable.setFont(this->m_font);
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
	target.draw(this->m_textDrawable, states);
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
	this->m_textDrawable.setFont(this->m_font);
	this->m_textDrawable.setFontBold(object.at("fontBold"));
	this->m_textDrawable.setFontSize(object.at("fontSize"));
	this->m_textDrawable.setText(object.at("text"));
	this->m_textDrawable.setRenderSize(object.at("renderSize"));
}
