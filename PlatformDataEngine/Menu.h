#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>

#include "Component.h"
#include "TextDrawable.h"
#include "InputManager.h"

namespace PlatformDataEngine {

	class ListMenu;
	class Menu;

	class MenuOption : public sf::Drawable, public sf::Transformable {

	public:
		inline MenuOption() {
			this->m_submenu = nullptr;
			this->m_isSelected = false;
			this->m_shader = std::make_shared<sf::Shader>();
			this->m_shader->loadFromFile("shaders/text.vert", "shaders/text.frag");
		};
		inline MenuOption(const std::string& text) {
			this->m_submenu = nullptr;
			this->m_isSelected = false;
			this->m_text.setText(text);
			this->m_shader = std::make_shared<sf::Shader>();
			this->m_shader->loadFromFile("shaders/text.vert", "shaders/text.frag");
		};

		virtual void init();
		virtual void update();

		inline void setSubmenu(ListMenu* submenu) { this->m_submenu = submenu; };
		inline ListMenu* getSubmenu() { return this->m_submenu; };

		void setSelected(bool selected);

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		inline void setFunctionPtr(void (*optFunction)(void)) { this->m_optFunction = optFunction; };
		virtual inline void interact() { this->m_optFunction(); };

	protected:
		TextDrawable m_text;
		bool m_isSelected;
		std::shared_ptr<sf::Shader> m_shader;
		ListMenu* m_submenu;
		
		void (*m_optFunction)(void);
	};

	class TextBox : public MenuOption {
		
	public:
		inline TextBox() {};
		TextBox(const std::string& defaultText, const std::string& label);

		void init();
		void update();

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void interact();

	private:
		TextDrawable m_labelText;
		sf::Sprite m_textBoxOutline;
	};

	class ListMenu : public Component
	{
	public:
		
		inline ListMenu() {};
		ListMenu(Menu* parent);

		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void addOption(std::shared_ptr<MenuOption> option, ListMenu* submenu);
		void addOption(std::shared_ptr<MenuOption> option, void (*optFunction)(void));

	private:
		
		std::vector<std::shared_ptr<MenuOption>> m_menuOptions;
		std::shared_ptr<InputManager> m_inputManager;
		bool m_inputActive;
		int m_optionCounter;
		int m_selectionIdx;
		MenuOption* m_currentlySelected;
		Menu* m_menuParent;
	};

	class Menu : public Component
	{
	public:

		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		inline void setCurrentMenu(ListMenu* currentMenu) { this->m_currentMenu = currentMenu; };
		inline ListMenu* getLastMenu() const { return this->m_lastMenu; };
		inline void setLastMenu(ListMenu* lastMenu) { this->m_lastMenu = lastMenu; };

		static inline void quit() {
			exit(0);
		}

	private:

		std::vector<std::shared_ptr<ListMenu>> m_menus;
		ListMenu* m_currentMenu;
		ListMenu* m_lastMenu;
	};

}