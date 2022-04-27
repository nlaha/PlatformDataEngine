#include "Menu.h"
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;


void Menu::init()
{
	std::shared_ptr<ListMenu> playerProfileMenu = std::make_shared<ListMenu>(this);
	playerProfileMenu->addOption(std::make_shared<TextBox>("Name:", playerProfileMenu.get()), &PlatformDataEngineWrapper::ProfileConfig::name);
	this->m_menus.push_back(playerProfileMenu);

	std::shared_ptr<ListMenu> optionsMenu = std::make_shared<ListMenu>(this);
	optionsMenu->addOption(std::make_shared<TextBox>("Music %:", optionsMenu.get()), &PlatformDataEngineWrapper::OptionsConfig::musicVol);
	this->m_menus.push_back(optionsMenu);

	// init host game menu
	std::shared_ptr<ListMenu> hostMenu = std::make_shared<ListMenu>(this);
	//hostMenu->addOption(std::make_shared<TextBox>("IP:", hostMenu.get()), &PlatformDataEngineWrapper::HostConfig::ip);
	hostMenu->addOption(std::make_shared<TextBox>("Port:", hostMenu.get()), &PlatformDataEngineWrapper::HostConfig::port);
	hostMenu->addOption(std::make_shared<MenuOption>("Start Server", hostMenu.get()), &PlatformDataEngineWrapper::loadServer);
	this->m_menus.push_back(hostMenu);

	// init join game menu
	std::shared_ptr<ListMenu> joinMenu = std::make_shared<ListMenu>(this);
	joinMenu->addOption(std::make_shared<TextBox>("IP:", joinMenu.get()), &PlatformDataEngineWrapper::JoinConfig::ip);
	joinMenu->addOption(std::make_shared<TextBox>("Port:", joinMenu.get()), &PlatformDataEngineWrapper::JoinConfig::port);
	joinMenu->addOption(std::make_shared<MenuOption>("Join Server", joinMenu.get()), &PlatformDataEngineWrapper::loadClient);
	this->m_menus.push_back(joinMenu);

	// init top level menu
	std::shared_ptr<ListMenu> topLevel = std::make_shared<ListMenu>(this);
	topLevel->addOption(std::make_shared<MenuOption>("Host Game", topLevel.get()), hostMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Join Game", topLevel.get()), joinMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Edit Profile", topLevel.get()), playerProfileMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Options", topLevel.get()), optionsMenu.get());
	topLevel->addOption(std::make_shared<MenuOption>("Quit", topLevel.get()), &PlatformDataEngineWrapper::quit);
	this->m_menus.push_back(topLevel);

	this->m_currentMenu = topLevel.get();

	m_bgMusic = PlatformDataEngineWrapper::getAudioSystem()->getMusic("menu.ogg");
	m_bgMusic->play();
	m_bgMusic->setVolume(30);
	m_bgMusic->setLoop(true);

}

void Menu::update(const float& dt, const float& elapsedTime)
{
	this->m_currentMenu->update(dt, elapsedTime);
	std::stringstream ss(PlatformDataEngineWrapper::OptionsConfig::musicVol);
	int vol = 30;
	ss >> vol;
	this->m_bgMusic->setVolume(vol);
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
	this->m_inputEditing = false;

	this->m_inputManager = PlatformDataEngineWrapper::getPlayerInputManager();
}

void ListMenu::init()
{
}

void ListMenu::update(const float& dt, const float& elapsedTime)
{
	if (this->m_inputManager) {
		if (this->m_inputManager->getAxis("y").isNegative() && !this->m_inputEditing) {
			if (!m_inputActive && !this->m_inputEditing) {
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
		else if (this->m_inputManager->getAxis("y").isPositive() && !this->m_inputEditing) {
			if (!m_inputActive && !this->m_inputEditing) {
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

	if (this->m_currentlySelected == nullptr)
	{
		this->m_currentlySelected = this->m_menuOptions.begin()->get();
		this->m_currentlySelected->setSelected(true);
	}
	else {
		this->m_currentlySelected->update(dt, elapsedTime);
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

void ListMenu::addOption(std::shared_ptr<TextBox> option, std::string* data)
{
	option->setPosition(option->getPosition().x, option->getPosition().y + (30 * m_optionCounter));
	m_optionCounter++;
	option->setScale({ 0.15f, 0.15f });
	option->setStringRef(data);
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

void MenuOption::update(const float& dt, const float& elapsedTime)
{
}

void MenuOption::setSelected(bool selected)
{
	this->m_isSelected = selected;
}

void MenuOption::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	this->m_shader->setUniform("selected", this->m_isSelected ? 1.0f : 0.0f);

	target.draw(this->m_text, states);
}

TextBox::TextBox(const std::string& label, ListMenu* parent)
{
	this->m_labelText.setText(label);
	this->m_labelText.setRenderSize(0.5f);
	this->m_parent = parent;
	this->m_isEditing = false;
}

void TextBox::init()
{
	this->m_labelText.setPosition({ -(this->m_text.getBounds().width / 2.0f), -65.0f});
	this->m_labelText.setScale({0.5f, 0.5f});
	this->m_text.setText(*this->m_stringPtr);
}

void TextBox::update(const float& dt, const float& elapsedTime)
{
	if (m_isEditing) {
		if (this->m_shader) {
			this->m_shader->setUniform("time", elapsedTime);
		}
		this->m_text.setText(PlatformDataEngineWrapper::getPlayerInput());
		*this->m_stringPtr = this->m_text.getText();
	}
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	if (this->m_shader) {
		this->m_shader->setUniform("selected", this->m_isSelected ? 1.0f : 0.0f);
		this->m_shader->setUniform("editing", this->m_isEditing ? 1.0f : 0.0f);
	}

	target.draw(this->m_text, states);
	target.draw(this->m_labelText, states);
}

void TextBox::interact()
{
	PlatformDataEngineWrapper::setPlayerInput(this->m_text.getText());
	this->m_isEditing = !this->m_isEditing;
	this->m_parent->setEditing(this->m_isEditing);
}
