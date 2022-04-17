#include "AudioSystem.h"

using namespace PlatformDataEngine;
namespace fs = std::filesystem;

AudioSystem::AudioSystem()
{
	// Load sounds
	const fs::path soundPath("./assets/audio/sounds/");

	for (const auto& entry : fs::directory_iterator(soundPath)) {
		const auto filenameStr = entry.path().filename().string();
		if (entry.is_regular_file()) {
			if (entry.path().extension() == ".ogg" || entry.path().extension() == ".wav")
			{
				// we've found an audio file
				spdlog::info("Loading sound file: {}", entry.path().string());

				std::shared_ptr<sf::SoundBuffer> buf = std::make_shared<sf::SoundBuffer>();
				buf->loadFromFile(entry.path().string());
				this->m_soundBuffers.push_back(buf);

				std::shared_ptr<sf::Sound> sound = std::make_shared<sf::Sound>();
				sound->setBuffer(*buf);
				m_sounds.emplace(filenameStr, sound);
			}
		}
	}

	// Load music
	const fs::path musicPath("./assets/audio/music/");

	for (const auto& entry : fs::directory_iterator(musicPath)) {
		const auto filenameStr = entry.path().filename().string();
		if (entry.is_regular_file()) {
			if (entry.path().extension() == ".ogg" || entry.path().extension() == ".wav")
			{
				// we've found an audio file
				spdlog::info("Loading music file: {}", entry.path().string());

				std::shared_ptr<sf::Music> music = std::make_shared<sf::Music>();
				music->openFromFile(entry.path().string());

				m_music.emplace(filenameStr, music);
			}
		}
	}
}

sf::Music* AudioSystem::getMusic(const std::string& name)
{
	return m_music.at(name).get();
}

sf::Sound* AudioSystem::getSound(const std::string& name)
{
	return m_sounds.at(name).get();
}
