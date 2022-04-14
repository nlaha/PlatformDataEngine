#include "Menu.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;


void Menu::init()
{
	// init host game menu
	std::shared_ptr<ListMenu> hostMenu = std::make_shared<ListMenu>(this);
	hostMenu->addOption(std::make_shared<TextBox>("localhost", "IP:"), &PlatformDataEngineWrapper::loadServer);
	hostMenu->addOption(std::make_shared<TextBox>("56656", "Port:"), &PlatformDataEngineWrapper::loadServer);
	hostMenu->addOption(std::make_shared<MenuOption>("Start Server"), &PlatformDataEngineWrapper::loadServer);
	this->m_menus.push_back(hostMenu);

	// init join game menu
	std::shared_ptr<ListMenu> joinMenu = std::make_shared<ListMenu>(this);
	joinMenu->addOption(std::make_shared<TextBox>("localhost", "IP:"), &PlatformDataEngineWrapper::loadClient);
	joinMenu->addOption(std::make_shared<TextBox>("56656", "Port:"), &PlatformDataEngineWrapper::loadClient);
	joinMenu->addOption(std::make_shared<MenuOption>("Join Server"), &PlatformDataEngineWrapper::loadClient);
	this->m_menus.push_back(joinMenu);

	// init top level menu
	std::shared_ptr<ListMenu> topLevel = std::make_shared<ListMenu>(this);
	topLevel->addOption(std::make_shared<MenuOption>("Host Game"), hostMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Join Game"), joinMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Quit"), &PlatformDataEngineWrapper::quit);
	this->m_menus.push_back(topLevel);

	this->m_currentMenu = topLevel.get();

}

void Menu::update(const float& dt, const float& elapsedTime)
{
	this->m_currentMenu->update(dt, elapsedTime);
}

void Menu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(*this->m_currentMenu, states);
}

ListMenu::ListMenu(Menu* parent)
{
	this->m_menuParent = parent;
	this->m_selectionIdx = 0;
	this->m_optionCounter = 0;
	this->m_currentlySelected = nullptr;
	this->m_inputActive = true;

	this->m_inputManager = PlatformDataEngineWrapper::getPlayerInputManager();
}

void ListMenu::init()
{
}

void ListMenu::update(const float& dt, const float& elapsedTime)
{
	if (this->m_currentlySelected == nullptr)
	{
		this->m_currentlySelected = this->m_menuOptions.begin()->get();
		this->m_currentlySelected->setSelected(true);
	}

	if (this->m_inputManager) {
		if (this->m_inputManager->getAxis("y").isNegative()) {
			if (!m_inputActive) {
				this->m_inputActive = true;
				this->m_currentlySelected->setSelected(false);
				if (this->m_selectionIdx < this->m_menuOptions.size() - 1)
				{
					this->m_selectionIdx++;
				}
				else {
					this->m_selectionIdx = 0;
				}
				this->m_currentlySelected = this->m_menuOptions[this->m_selectionIdx].get();
				this->m_currentlySelected->setSelected(true);
			}
		}
		else if (this->m_inputManager->getAxis("y").isPositive()) {
			if (!m_inputActive) {
				this->m_inputActive = true;
				this->m_currentlySelected->setSelected(false);
				if (this->m_selectionIdx > 0)
				{
					this->m_selectionIdx--;
				}
				else {
					this->m_selectionIdx = this->m_menuOptions.size() - 1;
				}
				this->m_currentlySelected = this->m_menuOptions[this->m_selectionIdx].get();
				this->m_currentlySelected->setSelected(true);
			}
		}
		else if (this->m_inputManager->getButton("interact").getValue()) {
			if (!m_inputActive) {
				this->m_inputActive = true;
				if (this->m_currentlySelected->getSubmenu() == nullptr) {
					this->m_currentlySelected->interact();
				}
				else {
					this->m_menuParent->setLastMenu(this);
					this->m_menuParent->setCurrentMenu(this->m_currentlySelected->getSubmenu());
				}
			}
		}
		else if (this->m_inputManager->getButton("back").getValue()) {
			if (!m_inputActive) {
				this->m_inputActive = true;
				if (this->m_menuParent->getLastMenu() != nullptr)
				{
					this->m_menuParent->setCurrentMenu(this->m_menuParent->getLastMenu());
				}
			}
		}
		else {
			this->m_inputActive = false;
		}
	}

	for (const auto& op : this->m_menuOptions)
	{
		op->update();
	}
}

void ListMenu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& op : this->m_menuOptions)
	{
		target.draw(*op, states);
	}
}


void ListMenu::addOption(std::shared_ptr<MenuOption> option, ListMenu* submenu)
{
	option->setPosition(option->getPosition().x, option->getPosition().y + (30 * m_optionCounter));
	m_optionCounter++;
	option->setScale({ 0.15f, 0.15f });
	option->setSubmenu(submenu);
	option->init();
	this->m_menuOptions.push_back(option);
}

void ListMenu::addOption(std::shared_ptr<MenuOption> option, void (*optFunction)(void))
{
	option->setPosition(option->getPosition().x, option->getPosition().y + (30 * m_optionCounter));
	m_optionCounter++;
	option->setScale({ 0.15f, 0.15f });
	option->setFunctionPtr(optFunction);
	option->init();
	this->m_menuOptions.push_back(option);
}

void MenuOption::init()
{
}

void MenuOption::update()
{
}

void MenuOption::setSelected(bool selected)
{
	//if (selected) {
	//	PlatformDataEngineWrapper::m_playerInput = this->m_text.getText();
	//}
	this->m_isSelected = selected;
}

void MenuOption::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.shader = this->m_shader.get();
	this->m_shader->setUniform("selected", this->m_isSelected ? 1.0f : 0.0f);

	target.draw(this->m_text, states);
}

TextBox::TextBox(const std::string& defaultText, const std::string& label)
{
	this->m_text.setText(defaultText);
	this->m_labelText.setText(label);
	this->m_labelText.setRenderSize(0.5f);
}

void TextBox::init()
{
	this->m_labelText.setPosition({ -(this->m_text.getBounds().width / 2.0f), -65.0f});
	this->m_labelText.setScale({0.5f, 0.5f});
}

void TextBox::update()
{
	//if (this->m_isSelected)
	//{
	//	this->m_text.setText(PlatformDataEngineWrapper::m_playerInput);
	//}
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.shader = this->m_shader.get();
	this->m_shader->setUniform("selected", this->m_isSelected ? 1.0f : 0.0f);

	target.draw(this->m_text, states);
	target.draw(this->m_labelText, states);
}

void TextBox::interact()
{

}
