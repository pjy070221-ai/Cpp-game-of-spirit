#include "PauseScene.h"
#include "GameplayScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include <SFML/Graphics.hpp>
#include <memory>

PauseScene::PauseScene() {
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    m_titleText.emplace(m_font, "Paused", 64);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 200.0f});

    for (auto& s : {"Continue", "Retry", "Return to Menu"}) {
        sf::Text txt(m_font, s, 36);
        txt.setFillColor(m_itemTexts.empty() ? sf::Color::Yellow : sf::Color(200, 200, 200));
        auto ib = txt.getLocalBounds();
        txt.setOrigin({ib.size.x / 2.0f, 0.0f});
        txt.setPosition({640.0f, 350.0f + m_itemTexts.size() * 90.0f});
        m_itemTexts.push_back(txt);
    }
}

void PauseScene::onEnter() {
    m_selection = 0;
    updateSelectionVisuals();
}

void PauseScene::handleEvent(const sf::Event& event) {
    // ── 键盘 ──
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::Up || key->code == sf::Keyboard::Key::Up) {
            m_selection = (m_selection + 1) % 3;
            updateSelectionVisuals();
            return;
        }
        if (key->scancode == sf::Keyboard::Scan::Down || key->code == sf::Keyboard::Key::Down) {
            m_selection = (m_selection + 1) % 3;
            updateSelectionVisuals();
            return;
        }
        if (key->scancode == sf::Keyboard::Scan::Enter || key->code == sf::Keyboard::Key::Enter) {
            activateItem(m_selection);
            return;
        }
        if (key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape) {
            activateItem(0);  // ESC = continue
            return;
        }
        return;
    }

    // ── 鼠标悬停 ──
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(mouseMoved->position);
        for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
            if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                if (m_selection != i) { m_selection = i; updateSelectionVisuals(); }
                return;
            }
        }
        return;
    }

    // ── 鼠标点击 ──
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
                if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                    activateItem(i);
                    return;
                }
            }
        }
        return;
    }
}

void PauseScene::activateItem(int index) {
    if (index == 0) {
        // Continue
        requestPop();
    } else if (index == 1) {
        // Retry
        GameplayScene::s_retry = true;
        requestPop();
    } else {
        // Return to Menu
        GameplayScene::s_returnToMenu = true;
        requestPop();
    }
}

void PauseScene::updateSelectionVisuals() {
    for (int i = 0; i < (int)m_itemTexts.size(); ++i)
        m_itemTexts[i].setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
}

void PauseScene::update(float) {}

void PauseScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20, 180));
    if (!m_fontLoaded) return;
    if (m_titleText.has_value()) target.draw(*m_titleText);
    for (auto& t : m_itemTexts) target.draw(t);
}


