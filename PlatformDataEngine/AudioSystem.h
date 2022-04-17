#pragma once
#include <vector>
#include <SFML/Audio.hpp>
#include <map>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <memory>
#include <string>

namespace PlatformDataEngine {
	class AudioSystem
	{

	public:

		AudioSystem();

		sf::Music* getMusic(const std::string& name);
		sf::Sound* getSound(const std::string& name);

	private:

		std::vector<std::shared_ptr<sf::SoundBuffer>> m_soundBuffers;
		std::map<std::string, std::shared_ptr<sf::Music>> m_music;
		std::map<std::string, std::shared_ptr<sf::Sound>> m_sounds;
	};
}
