#include "Application.h"
#include "SceneManager.h"

Application* Application::s_instance = nullptr;

Application::Application(const sf::String& title, unsigned int width, unsigned int height)
    : m_windowTitle(title), m_windowWidth(width), m_windowHeight(height)
{
    SettingsData settings;
    m_window.create(sf::VideoMode({width, height}), title, sf::Style::Default,
                  settings.getFullscreen() ? sf::State::Fullscreen : sf::State::Windowed);
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);
    s_instance = this;
    updateView();
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
    while (auto eventOpt = m_window.pollEvent()) {
        sf::Event event = *eventOpt;

        if (event.is<sf::Event::Closed>()) {
            m_window.close();
        } else if (event.is<sf::Event::Resized>()) {
            updateView();
        } else {
            if (auto* mm = event.getIf<sf::Event::MouseMoved>()) {
                sf::Vector2f world = m_window.mapPixelToCoords(mm->position);
                mm->position = sf::Vector2i(static_cast<int>(world.x), static_cast<int>(world.y));
            } else if (auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f world = m_window.mapPixelToCoords(mb->position);
                mb->position = sf::Vector2i(static_cast<int>(world.x), static_cast<int>(world.y));
            } else if (auto* mbr = event.getIf<sf::Event::MouseButtonReleased>()) {
                sf::Vector2f world = m_window.mapPixelToCoords(mbr->position);
                mbr->position = sf::Vector2i(static_cast<int>(world.x), static_cast<int>(world.y));
            }
            m_sceneManager.handleEvent(event);
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

void Application::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        auto mode = sf::VideoMode::getDesktopMode();
        m_window.create(mode, m_windowTitle, sf::Style::Default, sf::State::Fullscreen);
    } else {
        m_window.create(sf::VideoMode({m_windowWidth, m_windowHeight}), m_windowTitle, sf::Style::Default, sf::State::Windowed);
    }
    m_window.setVerticalSyncEnabled(true);
    m_window.setKeyRepeatEnabled(false);
    updateView();
}

void Application::updateView() {
    float gameAspect = 1280.0f / 720.0f;
    auto winSize = m_window.getSize();
    if (winSize.x == 0 || winSize.y == 0) return;
    float windowAspect = static_cast<float>(winSize.x) / static_cast<float>(winSize.y);

    sf::View view(sf::FloatRect({0, 0}, {1280.0f, 720.0f}));

    if (windowAspect > gameAspect) {
        float scale = gameAspect / windowAspect;
        view.setViewport(sf::FloatRect({(1.0f - scale) / 2.0f, 0.0f}, {scale, 1.0f}));
    } else {
        float scale = windowAspect / gameAspect;
        view.setViewport(sf::FloatRect({0.0f, (1.0f - scale) / 2.0f}, {1.0f, scale}));
    }

    m_window.setView(view);
}

sf::RenderWindow& Application::getWindow() { return m_window; }
SceneManager&     Application::getSceneManager() { return m_sceneManager; }
float             Application::getDeltaTime() const { return m_deltaTime; }

