#include "Application.h"
#include "SceneManager.h"

Application::Application(const sf::String& title, unsigned int width, unsigned int height)
    : m_window(sf::VideoMode({width, height}), title, sf::Style::Default)
{
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);
}

void Application::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        if (SceneManager::s_exitRequested) {
            m_window.close();
            break;
        }
        m_deltaTime = clock.restart().asSeconds();
        if (m_deltaTime > 0.05f) m_deltaTime = 0.05f;

        processEvents();
        update();
        render();
    }
}

void Application::processEvents() {
    while (const std::optional<sf::Event> event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else {
            m_sceneManager.handleEvent(*event);
        }
    }
}

void Application::update() {
    if (!m_sceneManager.isEmpty()) {
        m_sceneManager.update(m_deltaTime);
    }
}

void Application::render() {
    m_window.clear(sf::Color(10, 5, 20));
    if (!m_sceneManager.isEmpty()) {
        m_sceneManager.render(m_window);
    }
    m_window.display();
}

sf::RenderWindow& Application::getWindow() { return m_window; }
SceneManager&     Application::getSceneManager() { return m_sceneManager; }
float             Application::getDeltaTime() const { return m_deltaTime; }

