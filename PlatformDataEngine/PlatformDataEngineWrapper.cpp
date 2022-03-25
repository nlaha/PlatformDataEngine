#include "PlatformDataEngineWrapper.h"

namespace fs = std::filesystem;

namespace PlatformDataEngine {

    std::shared_ptr<GameWorld> PlatformDataEngineWrapper::mp_mainWorld = std::make_shared<GameWorld>();

    PlatformDataEngineWrapper::PlatformDataEngineWrapper()
    {
    }

    PlatformDataEngineWrapper::~PlatformDataEngineWrapper()
    {
    }

    /// <summary>
    /// Runs the game (makes window and starts systems)
    /// </summary>
    void PlatformDataEngineWrapper::run()
    {
        sf::ContextSettings contextSettings;

        // create window and viewport
        sf::RenderWindow window(sf::VideoMode(1920, 1024), "PlatformData Engine", sf::Style::Default, contextSettings);
        sf::FloatRect visibleArea(0.f, 0.f, 256, 256);
        sf::View gameView(visibleArea);
        float xoffset = ((window.getSize().x - window.getSize().y) / 2.0f) / window.getSize().x;
        sf::FloatRect viewPort = sf::FloatRect({ xoffset, 0 }, { (float)window.getSize().y / (float)window.getSize().x, 1.0f });
        gameView.setViewport(viewPort);
        window.setView(gameView);

        bool isFullscreen = false;

        mp_mainWorld->initPhysics();

        // TODO: load game objects definitions from gameObjects/*.json
        // loop through all json files in game/gameObjects
        // and create game objects from them
        const fs::path gameObjectPath("./game/gameObjects/");

        for (const auto& entry : fs::directory_iterator(gameObjectPath)) {
            const auto filenameStr = entry.path().filename().string();
            if (entry.is_regular_file()) {
                if (entry.path().extension() == ".json")
                {
                    // we've found a gameObject definition
                    spdlog::info("Loading GameObject definition: {}", entry.path().string());
                    GameObject gameObject;
                    gameObject.loadDefinition(entry.path().string());
                    mp_mainWorld->registerGameObjectDefinition(entry.path().filename().replace_extension("").string(), gameObject);
                }
            }
        }

        // init main world
        mp_mainWorld->init("game/world.json", gameView);

        // game loop
        sf::Clock fpsClock;
        sf::Clock deltaClock;
        sf::Clock elapsedClock;
        sf::Time dt;
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();

                // catch the resize events
                if (event.type == sf::Event::Resized)
                {
                    // update the view to the new size of the window
                    float xoffset = ((event.size.width - event.size.height) / 2.0f) / event.size.width;
                    viewPort = sf::FloatRect({ xoffset, 0 }, { (float)event.size.height / (float)event.size.width, 1.0f });
                }

                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::F11 || 
                        event.key.alt && event.key.code == sf::Keyboard::Enter)
                    {
                        window.close();
                        if (!isFullscreen) {
                            window.create(sf::VideoMode::getDesktopMode(), "PlatformData Engine", sf::Style::None, contextSettings);
                            isFullscreen = true;
                        }
                        else {
                            window.create(sf::VideoMode(1920, 1024), "PlatformData Engine", sf::Style::Default, contextSettings);
                            isFullscreen = false;
                        }
                    }
                }
            }

            // always top left of window (for GUI)
            this->m_windowZero = window.mapPixelToCoords({ 0, 0 });

            mp_mainWorld->update(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update world

            mp_mainWorld->physicsUpdate(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update physics world

            // update view
            sf::View view = mp_mainWorld->getView();
            view.setViewport(viewPort);
            window.setView(view);

            window.clear(sf::Color(0, 0, 0));

            window.draw(*mp_mainWorld);

            // get delta time
            dt = deltaClock.restart();

            // calculate fps
            this->m_fps = 1.f / fpsClock.getElapsedTime().asSeconds();
            fpsClock.restart();

            // print some stats
            spdlog::debug("FPS: {0:.2f} --- DT: {1:.2f}", this->m_fps, dt.asSeconds());

            window.display();
        }
    }
}