#pragma once
#include "Component.h"

namespace PlatformDataEngine {

	/// <summary>
	/// A very basic text rendering component
	/// </summary>
	class TextRenderer : public Component
	{
	public:
		TextRenderer();

		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void copy(std::shared_ptr<Component> otherCompPtr);

		void loadDefinition(nlohmann::json object);

		inline void setText(std::string text) { this->m_text = text; };
		inline std::string getText() const { this->m_text; };

	private:
		sf::Font m_font;
		int m_fontSize;
		bool m_fontBold;
		float m_renderSize;
		std::string m_text;
	};
}
