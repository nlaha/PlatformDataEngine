#pragma once
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine
{

	/// <summary>
	/// A more generic text render drawable
	/// that can be used outside a component context
	/// </summary>
	class TextDrawable : public sf::Drawable, public sf::Transformable
	{
	public:
		TextDrawable(std::string fontFilename = "assets/smallest_pixel-7.ttf");
		TextDrawable(int fontSize, bool bold, float renderSize, const std::string &text);

		void setText(const std::string &text);

		void draw(sf::RenderTarget &target, sf::RenderStates states) const;

		// getters

		inline sf::Font& getFont() { return this->m_font; };
		inline int getFontSize() const { return this->m_fontSize; };
		inline bool getFontBold() const { return this->m_fontBold; };
		inline float getRenderSize() const { return this->m_renderSize; };
		inline const std::string& getText() const { return this->m_text; };
		inline const sf::FloatRect getBounds() const { return this->m_quads.getBounds(); };
		inline std::shared_ptr<sf::Shader> getShader() const { return this->m_shader; };

		// setters

		inline void setFont(const sf::Font& font) { this->m_font = font; };
		inline void setFontSize(int fontSize) { this->m_fontSize = fontSize; };
		inline void setFontBold(bool bold) { this->m_fontBold = m_fontBold; };
		inline void setRenderSize(float renderSize) { this->m_renderSize = renderSize; };
		inline void setColor(const sf::Color& color) {
			this->m_shader->setUniform("textColor", sf::Glsl::Vec4(color));
		}

	private:
		sf::Font m_font;
		int m_fontSize;
		bool m_fontBold;
		float m_renderSize;
		std::string m_text;
		sf::VertexArray m_quads;
		bool m_isCentered;
		std::shared_ptr<sf::Shader> m_shader;
	};
}
