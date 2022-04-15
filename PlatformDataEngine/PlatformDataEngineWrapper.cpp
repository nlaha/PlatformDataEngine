#include "PlatformDataEngineWrapper.h"

namespace fs = std::filesystem;

namespace PlatformDataEngine {

    std::shared_ptr<GameWorld> PlatformDataEngineWrapper::mp_mainWorld = nullptr;
    std::shared_ptr<PlayerInputManager> PlatformDataEngineWrapper::mp_playerInputManager = std::make_shared<PlayerInputManager>(0);
    std::shared_ptr<sf::RenderWindow> PlatformDataEngineWrapper::mp_renderWindow = nullptr;
    bool PlatformDataEngineWrapper::m_pausedGame = false;
    bool PlatformDataEngineWrapper::m_debugPhysics = false;
    bool PlatformDataEngineWrapper::m_isClient = false;
    std::string PlatformDataEngineWrapper::m_playerName = "Player";
    sf::View PlatformDataEngineWrapper::m_view;
    std::thread PlatformDataEngineWrapper::m_renderThread;
    std::atomic<bool> PlatformDataEngineWrapper::m_renderThreadStop(false);
    std::string PlatformDataEngineWrapper::m_playerInput = "";
    std::shared_ptr<PhysicsDebugDraw> PlatformDataEngineWrapper::m_debugDraw = nullptr;

    std::shared_ptr <NetworkHandler> PlatformDataEngineWrapper::m_netHandler = nullptr;

    PlatformDataEngineWrapper::PlatformDataEngineWrapper()
    {
    }

    PlatformDataEngineWrapper::~PlatformDataEngineWrapper()
    {
    }

    void renderingThread(std::shared_ptr<sf::RenderWindow> window, GameWorld* world, std::atomic<bool>& threadStop)
    {
        sf::Clock fpsClock;
        while (window->isOpen() && !threadStop.load(std::memory_order_relaxed))
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
            //spdlog::debug("FPS: {0:.2f}", PlatformDataEngineWrapper::m_fps);
        }
    }

    /// <summary>
    /// Runs the game (makes window and starts systems)
    /// </summary>
    void PlatformDataEngineWrapper::run(ApplicationMode appMode)
    {
        m_isClient = appMode == ApplicationMode::CLIENT ? true : false;

        sf::ContextSettings contextSettings;

        std::string flags = " [";

        if (m_isClient) {
            flags += "Client]";
        }
        else {
            flags += "Server]";
        }

        // create window and viewport
        sf::FloatRect viewPort;
        if (appMode != ApplicationMode::DEDICATED) {
            mp_renderWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode(640, 640), "PlatformData Engine" + flags, sf::Style::Default, contextSettings);
            sf::FloatRect visibleArea(0.f, 0.f, 256, 256);
            m_view = sf::View(visibleArea);
            float xoffset = ((mp_renderWindow->getSize().x - mp_renderWindow->getSize().y) / 2.0f) / mp_renderWindow->getSize().x;
            viewPort = sf::FloatRect({ xoffset, 0 }, { (float)mp_renderWindow->getSize().y / (float)mp_renderWindow->getSize().x, 1.0f });
            m_view.setViewport(viewPort);
            mp_renderWindow->setView(m_view);
        }

        bool isFullscreen = false;

        // init input
        mp_playerInputManager->loadDefinition("./game/input.json");

        // load game worlds
        mp_mainWorld = std::make_shared<GameWorld>();

        // init physics
        mp_mainWorld->initPhysics();

        mp_mainWorld->loadGameObjectDefinitions();

        if (appMode != ApplicationMode::DEDICATED) {
            // show menu
            mp_mainWorld->init("game/worlds/menu.json", m_view, appMode);
        }
        else {
            mp_mainWorld->init("game/worlds/world.json", m_view, appMode);
        }

        // game loop
        sf::Clock deltaClock;
        sf::Clock elapsedClock;
        sf::Time dt;

        if (appMode != ApplicationMode::DEDICATED) {
            // deactivate its OpenGL context
            mp_renderWindow->setActive(false);

            PlatformDataEngineWrapper::startRenderThread();
        }

        while (appMode == ApplicationMode::DEDICATED || mp_renderWindow->isOpen())
        {
            sf::Event event;
            if (appMode != ApplicationMode::DEDICATED) {
                while (mp_renderWindow->pollEvent(event))
                {
                    if (event.type == sf::Event::TextEntered)
                    {
                        if (event.text.unicode < 128)
                        {
                            PlatformDataEngineWrapper::m_playerInput += event.text.unicode;
                        }
                    }
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
                            m_renderThread.join();
                            if (!isFullscreen) {
                                mp_renderWindow->create(sf::VideoMode::getDesktopMode(), "PlatformData Engine", sf::Style::None, contextSettings);
                                isFullscreen = true;
                            }
                            else {
                                mp_renderWindow->create(sf::VideoMode(1920, 1024), "PlatformData Engine", sf::Style::Default, contextSettings);
                                isFullscreen = false;
                            }
                            PlatformDataEngineWrapper::startRenderThread();
                        }
                        else if (event.key.code == sf::Keyboard::Pause)
                        {
                            m_pausedGame = !m_pausedGame;
                        }
                        else if (event.key.code == sf::Keyboard::Home)
                        {
                            m_debugPhysics = !m_debugPhysics;
                        }
                        else if (event.key.code == sf::Keyboard::BackSpace)
                        {
                            if (PlatformDataEngineWrapper::m_playerInput.size() > 0) {
                                PlatformDataEngineWrapper::m_playerInput.pop_back();
                            }
                        }
                    }

                    if (event.type == sf::Event::JoystickMoved)
                    {

                    }
                }
            }

            // update view
            if (appMode != ApplicationMode::DEDICATED) {

                // always top left of window (for GUI)
                this->m_windowZero = mp_renderWindow->mapPixelToCoords({ 0, 0 });

                sf::View view = mp_mainWorld->getView();
                view.setViewport(viewPort);
                mp_renderWindow->setView(view);
            }
            if (!m_pausedGame) {
                mp_mainWorld->update(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update world
                mp_mainWorld->physicsUpdate(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update physics world

                // get delta time
                dt = deltaClock.restart();
            }
        }

        if (m_isClient) {
            dynamic_cast<Client*>(PlatformDataEngineWrapper::getNetworkHandler())->disconnect();
        }
        if(m_renderThread.joinable()) {
            m_renderThread.join();
        }
    }
}