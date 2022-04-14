#pragma once
#include "Component.h"
#include "TextDrawable.h"

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

		inline void setText(const std::string& text) {
			this->m_textDrawable.setText(text);
		};

		inline std::string getText() const { this->m_textDrawable.getText(); };

	private:
		sf::Font m_font;

		TextDrawable m_textDrawable;
	};
}
