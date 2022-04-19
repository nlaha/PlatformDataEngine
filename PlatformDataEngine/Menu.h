#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>

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
			this->m_shader = this->m_text.getShader();
		};

		inline MenuOption(ListMenu* parent) {
			this->m_submenu = nullptr;
			this->m_isSelected = false;
			this->m_shader = this->m_text.getShader();
			this->m_parent = parent;
		};
		inline MenuOption(const std::string& text, ListMenu* parent) {
			this->m_submenu = nullptr;
			this->m_isSelected = false;
			this->m_text.setText(text);
			this->m_shader = this->m_text.getShader();
			this->m_parent = parent;
		};

		virtual void init();
		virtual void update(const float& dt, const float& elapsedTime);

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
		ListMenu* m_parent;

		void (*m_optFunction)(void);
	};

	class TextBox : public MenuOption {
		
	public:
		inline TextBox() { this->m_isEditing = false; };
		TextBox(const std::string& label, ListMenu* parent);

		inline void setStringRef(std::string* stringPtr) { this->m_stringPtr = stringPtr; };

		void init();
		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void interact();

	private:
		TextDrawable m_labelText;
		sf::Sprite m_textBoxOutline;

		std::string* m_stringPtr;
		bool m_isEditing;
	};

	class ListMenu : public Component
	{
	public:
		
		inline ListMenu() {};
		ListMenu(Menu* parent);

		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		void addOption(std::shared_ptr<TextBox> option, std::string* data);
		void addOption(std::shared_ptr<MenuOption> option, ListMenu* submenu);
		void addOption(std::shared_ptr<MenuOption> option, void (*optFunction)(void));

		inline void setEditing(bool editing) { this->m_inputEditing = editing; };

	private:
		
		std::vector<std::shared_ptr<MenuOption>> m_menuOptions;
		std::shared_ptr<InputManager> m_inputManager;
		bool m_inputActive;
		int m_optionCounter;
		int m_selectionIdx;
		MenuOption* m_currentlySelected;
		Menu* m_menuParent;
		bool m_inputEditing;
	};

	class Menu : public Component
	{
	public:

		void init();

		void update(const float& dt, const float& elapsedTime);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const;

		inline ListMenu* getLastMenu() const { return this->m_lastMenu; };

		inline void setCurrentMenu(ListMenu* currentMenu) { this->m_currentMenu = currentMenu; };
		inline void setLastMenu(ListMenu* lastMenu) { this->m_lastMenu = lastMenu; };

	private:

		std::vector<std::shared_ptr<ListMenu>> m_menus;
		ListMenu* m_currentMenu;
		ListMenu* m_lastMenu;
		sf::Music* m_bgMusic;
	};

}