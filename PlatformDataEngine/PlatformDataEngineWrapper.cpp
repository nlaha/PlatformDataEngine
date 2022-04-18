#include "PlatformDataEngineWrapper.h"

namespace fs = std::filesystem;

namespace PlatformDataEngine {

    std::shared_ptr<GameWorld> PlatformDataEngineWrapper::mp_mainWorld = nullptr;
    std::shared_ptr<PlayerInputManager> PlatformDataEngineWrapper::mp_playerInputManager = std::make_shared<PlayerInputManager>(0);
    std::shared_ptr<sf::RenderWindow> PlatformDataEngineWrapper::mp_renderWindow = nullptr;
    bool PlatformDataEngineWrapper::m_pausedGame = false;
    bool PlatformDataEngineWrapper::m_debugPhysics = false;
    bool PlatformDataEngineWrapper::m_isClient = false;
    sf::View PlatformDataEngineWrapper::m_view;
    std::shared_ptr<sf::Thread> PlatformDataEngineWrapper::m_renderThread;
    std::atomic<bool> PlatformDataEngineWrapper::m_renderThreadStop(false);
    std::string PlatformDataEngineWrapper::m_playerInput = "";
    std::shared_ptr<PhysicsDebugDraw> PlatformDataEngineWrapper::m_debugDraw = nullptr;
    std::shared_ptr <NetworkHandler> PlatformDataEngineWrapper::m_netHandler = nullptr;
    sf::FloatRect PlatformDataEngineWrapper::m_viewPort = sf::FloatRect();
    sf::Vector2f PlatformDataEngineWrapper::m_windowZero = sf::Vector2f(0.0f, 0.0f);
    sf::Vector2f PlatformDataEngineWrapper::m_windowCenter = sf::Vector2f(0.0f, 0.0f);

    std::string PlatformDataEngineWrapper::HostConfig::ip = "localhost";
    std::string PlatformDataEngineWrapper::HostConfig::port = "65525";

    std::string PlatformDataEngineWrapper::JoinConfig::ip = "localhost";
    std::string PlatformDataEngineWrapper::JoinConfig::port = "65525";

    std::string PlatformDataEngineWrapper::ProfileConfig::name = "Player";

    std::shared_ptr<AudioSystem> PlatformDataEngineWrapper::m_audioSystem = std::make_shared<AudioSystem>();

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

            sf::View view = world->getView();
            view.setViewport(PlatformDataEngineWrapper::getViewport());
            window->setView(view);

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

        // create window and viewport
        if (appMode != ApplicationMode::DEDICATED) {
            mp_renderWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode(640, 640), "PlatformData Engine", sf::Style::Default, contextSettings);
            sf::FloatRect visibleArea(0.f, 0.f, 256, 256);
            m_view = sf::View(visibleArea);
            float xoffset = ((mp_renderWindow->getSize().x - mp_renderWindow->getSize().y) / 2.0f) / mp_renderWindow->getSize().x;
            m_viewPort = sf::FloatRect({ xoffset, 0 }, { (float)mp_renderWindow->getSize().y / (float)mp_renderWindow->getSize().x, 1.0f });
            m_view.setViewport(m_viewPort);
            mp_renderWindow->setView(m_view);
            //mp_renderWindow->setVerticalSyncEnabled(true);
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
            if (!m_pausedGame) {
                mp_mainWorld->update(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update world
                mp_mainWorld->physicsUpdate(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update physics world

                // get delta time
                dt = deltaClock.restart();
            }

            sf::Event event;
            if (appMode != ApplicationMode::DEDICATED) {
                while (mp_renderWindow->pollEvent(event))
                {
                    if (event.type == sf::Event::TextEntered)
                    {
                        if (event.text.unicode < 128 && event.text.unicode != '\b' && event.text.unicode != 13)
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
                        m_viewPort = sf::FloatRect({ xoffset, 0 }, { (float)event.size.height / (float)event.size.width, 1.0f });

                    }

                    if (event.type == sf::Event::KeyPressed) {
                        if (event.key.code == sf::Keyboard::F11 ||
                            event.key.alt && event.key.code == sf::Keyboard::Enter)
                        {
                            mp_renderWindow->close();
                            m_renderThread->wait();
                            if (!isFullscreen) {
                                mp_renderWindow->create(sf::VideoMode::getDesktopMode(), "PlatformData Engine", sf::Style::None, contextSettings);
                                isFullscreen = true;

                                // update the view to the new size of the window
                                float xoffset = ((sf::VideoMode::getDesktopMode().width - sf::VideoMode::getDesktopMode().height) / 2.0f) / sf::VideoMode::getDesktopMode().width;
                                m_viewPort = sf::FloatRect({ xoffset, 0 }, { (float)sf::VideoMode::getDesktopMode().height / (float)sf::VideoMode::getDesktopMode().width, 1.0f });
                            }
                            else {
                                mp_renderWindow->create(sf::VideoMode(640, 640), "PlatformData Engine", sf::Style::Default, contextSettings);
                                isFullscreen = false;

                                m_viewPort = sf::FloatRect({ 0, 0 }, { (float)640 / (float)640, 1.0f });
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
                        else if (event.key.code == sf::Keyboard::Backspace)
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
                PlatformDataEngineWrapper::m_windowZero = mp_renderWindow->mapPixelToCoords({ 0, 0 });
                sf::Vector2i center = sf::Vector2i(
                    mp_renderWindow->getSize().x / 2,
                    mp_renderWindow->getSize().y / 2);
                PlatformDataEngineWrapper::m_windowCenter = mp_renderWindow->mapPixelToCoords(center);

            }
        }

        if (m_isClient) {
            dynamic_cast<Client*>(PlatformDataEngineWrapper::getNetworkHandler())->disconnect();
        }
        m_renderThread->wait();
    }
}