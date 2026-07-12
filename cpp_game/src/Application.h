#pragma once

#include <SFML/Graphics.hpp>
#include "SceneManager.h"

class Application {
public:
    Application(const sf::String& title, unsigned int width, unsigned int height);
    ~Application() = default;

    void run();

    sf::RenderWindow& getWindow();
    SceneManager& getSceneManager();
    float getDeltaTime() const;

private:
    void processEvents();
    void update();
    void render();

    sf::RenderWindow m_window;
    SceneManager m_sceneManager;
    sf::Clock m_clock;
    float m_deltaTime = 0.0f;
};
