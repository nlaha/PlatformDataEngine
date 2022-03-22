#include "PlatformDataEngine.h"

namespace fs = std::filesystem;

namespace PlatformDataEngine {

    std::shared_ptr<GameWorld> PlatformDataEngine::mp_mainWorld = std::make_shared<GameWorld>();

    PlatformDataEngine::PlatformDataEngine()
    {
    }

    PlatformDataEngine::~PlatformDataEngine()
    {
    }

    void PlatformDataEngine::run()
    {
        sf::ContextSettings contextSettings;
        contextSettings.antialiasingLevel = 0;

        // create window and viewport
        sf::RenderWindow window(sf::VideoMode(1920, 1024), "PlatformData Engine", 7U, contextSettings);
        sf::FloatRect visibleArea(0.f, 0.f, 256, 256);
        sf::View gameView(visibleArea);
        float xoffset = ((window.getSize().x - window.getSize().y) / 2.0f) / window.getSize().x;
        gameView.setViewport(sf::FloatRect({ xoffset, 0 }, { (float)window.getSize().y / (float)window.getSize().x, 1.0f }));
        window.setView(gameView);

        // load pixel font
        sf::Font font;
        if (!font.loadFromFile("assets/VT323-Regular.ttf"))
        {
            spdlog::error("Could not load font!");
        }

        // always top left of window (for GUI)
        sf::Vector2f windowZero = window.mapPixelToCoords({ 0, 0 });

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
        sf::Clock clock;
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
                    gameView.setViewport(sf::FloatRect({ xoffset, 0 }, { (float)event.size.height / (float)event.size.width, 1.0f }));
                    window.setView(gameView);
                }

                // just for testing
                // if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                //{
                //    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                //    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                //    TileSprite* sprite1 = new TileSprite(worldTs, 45, worldPos);
                //    sprite1->scale({ 1, 1 });
                //    std::cout << "Created Sprite" << std::endl;
                //}
            }

            mp_mainWorld->update(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update world

            mp_mainWorld->physicsUpdate(dt.asSeconds(), elapsedClock.getElapsedTime().asSeconds()); // update physics world

            // update view
            window.setView(mp_mainWorld->getView());

            window.clear(sf::Color(0, 0, 0));

            window.draw(*mp_mainWorld);

            // get delta time
            dt = deltaClock.restart();

            // calculate fps
            float fps = 1.f / clock.getElapsedTime().asSeconds();
            clock.restart();

            // print some stats
            spdlog::debug("FPS: {0:.2f} --- DT: {1:.2f}", fps, dt.asSeconds());

            window.display();
        }
    }
}