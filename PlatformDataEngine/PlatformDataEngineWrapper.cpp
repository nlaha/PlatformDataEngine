#include "PlatformDataEngineWrapper.h"

namespace fs = std::filesystem;

namespace PlatformDataEngine {

    std::shared_ptr<GameWorld> PlatformDataEngineWrapper::mp_mainWorld = std::make_shared<GameWorld>();
    std::shared_ptr<PlayerInputManager> PlatformDataEngineWrapper::mp_playerInputManager = std::make_shared<PlayerInputManager>(0);
    std::shared_ptr<sf::RenderWindow> PlatformDataEngineWrapper::mp_renderWindow = nullptr;
    bool PlatformDataEngineWrapper::m_pausedGame = false;
    bool PlatformDataEngineWrapper::m_debugPhysics = false;

    PlatformDataEngineWrapper::PlatformDataEngineWrapper()
    {
    }

    PlatformDataEngineWrapper::~PlatformDataEngineWrapper()
    {
    }

    void renderingThread(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<GameWorld> world)
    {
        sf::Clock fpsClock;
        while (window->isOpen())
        {
            // clear
            window->clear(sf::Color(0, 0, 0));

            // draw...
            window->draw(*world);

            // end the current frame
            window->display();


            // calculate fps
            PlatformDataEngineWrapper::m_fps = 1.f / fpsClock.getElapsedTime().asSeconds();
            fpsClock.restart();

            // print some stats
            spdlog::debug("FPS: {0:.2f}", PlatformDataEngineWrapper::m_fps);
        }
    }

    /// <summary>
    /// Runs the game (makes window and starts systems)
    /// </summary>
    void PlatformDataEngineWrapper::run()
    {
        sf::ContextSettings contextSettings;

        // create window and viewport
        mp_renderWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode(1920, 1024), "PlatformData Engine", sf::Style::Default, contextSettings);
        sf::FloatRect visibleArea(0.f, 0.f, 256, 256);
        sf::View gameView(visibleArea);
        float xoffset = ((mp_renderWindow->getSize().x - mp_renderWindow->getSize().y) / 2.0f) / mp_renderWindow->getSize().x;
        sf::FloatRect viewPort = sf::FloatRect({ xoffset, 0 }, { (float)mp_renderWindow->getSize().y / (float)mp_renderWindow->getSize().x, 1.0f });
        gameView.setViewport(viewPort);
        mp_renderWindow->setView(gameView);

        bool isFullscreen = false;

        // init input
        mp_playerInputManager->loadDefinition("./game/input.json");

        // init physics
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
                    std::shared_ptr<GameObject> gameObject = std::make_shared<GameObject>(true);
                    gameObject->loadDefinition(entry.path().string());
                    mp_mainWorld->registerGameObjectDefinition(entry.path().filename().replace_extension("").string(), gameObject);
                }
            }
        }

        // init main world
        mp_mainWorld->init("game/world.json", gameView);

        // game loop
        sf::Clock deltaClock;
        sf::Clock elapsedClock;
        sf::Time dt;

        /* Initialize Debug Draw */
        PhysicsDebugDraw debugDraw(*mp_renderWindow);

        mp_mainWorld->getPhysWorld()->SetDebugDraw(&debugDraw);
        debugDraw.SetFlags(b2Draw::e_shapeBit); //Only draw shapes

        // deactivate its OpenGL context
        mp_renderWindow->setActive(false);

        std::thread renderThread(&renderingThread, mp_renderWindow, mp_mainWorld);

        while (mp_renderWindow->isOpen())
        {
            sf::Event event;
            while (mp_renderWindow->pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    mp_renderWindow->close();

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
                        mp_renderWindow->close();
                        renderThread.join();
                        if (!isFullscreen) {
                            mp_renderWindow->create(sf::VideoMode::getDesktopMode(), "PlatformData Engine", sf::Style::None, contextSettings);
                            isFullscreen = true;
                        }
                        else {
                            mp_renderWindow->create(sf::VideoMode(1920, 1024), "PlatformData Engine", sf::Style::Default, contextSettings);
                            isFullscreen = false;
                        }
                        mp_renderWindow->setActive(false);
                        renderThread = std::thread(&renderingThread, mp_renderWindow, mp_mainWorld);
                    }
                    else if (event.key.code == sf::Keyboard::Pause)
                    {
                        m_pausedGame = !m_pausedGame;
                    }
                    else if (event.key.code == sf::Keyboard::Home)
                    {
                        m_debugPhysics = !m_debugPhysics;
                    }
                }
            }

            // always top left of window (for GUI)
            this->m_windowZero = mp_renderWindow->mapPixelToCoords({ 0, 0 });

            if (!m_pausedGame) {
                mp_mainWorld->update(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update world
                mp_mainWorld->physicsUpdate(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update physics world
            }
            
            // update view
            sf::View view = mp_mainWorld->getView();
            view.setViewport(viewPort);
            mp_renderWindow->setView(view);

            //mp_renderWindow->draw(*mp_mainWorld);

            // get delta time
            dt = deltaClock.restart();

        }
        renderThread.join();
    }
}