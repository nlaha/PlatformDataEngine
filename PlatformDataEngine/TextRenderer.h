#pragma once
#include "Component.h"

namespace PlatformDataEngine {

	class TextRenderer : public Component
	{
	public:
		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void loadDefinition(nlohmann::json object);

		inline void setText(std::string text) { this->m_text = text; };
		inline std::string getText() const { this->m_text; };

	private:
		std::shared_ptr<sf::Font> m_font;
		int m_fontSize;
		bool m_fontBold;
		float m_renderSize;
		std::string m_text;
	};
}
