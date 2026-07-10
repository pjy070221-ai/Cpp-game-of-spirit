#include "MenuScene.h"
#include "PackScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <memory>

MenuScene::MenuScene() : m_items({"Start Game", "Settings", "Exit"}) {
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    m_titleText.emplace(m_font, "Cross Beat", 72);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 150.0f});

    for (int i = 0; i < (int)m_items.size(); ++i) {
        sf::Text txt(m_font, m_items[i], 36);
        txt.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
        auto ib = txt.getLocalBounds();
        txt.setOrigin({ib.size.x / 2.0f, 0.0f});
        txt.setPosition({640.0f, 330.0f + i * 80.0f});
        m_itemTexts.push_back(txt);
    }
}

void MenuScene::onEnter() {
    m_selection = 0;
    updateSelectionVisuals();
}

void MenuScene::handleEvent(const sf::Event& event) {
    // ── 键盘导航 ──
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        bool changed = false;
        if (key->scancode == sf::Keyboard::Scan::Up || key->code == sf::Keyboard::Key::Up) {
            m_selection = (m_selection - 1 + (int)m_items.size()) % m_items.size();
            changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Down || key->code == sf::Keyboard::Key::Down) {
            m_selection = (m_selection + 1) % m_items.size();
            changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Enter || key->code == sf::Keyboard::Key::Enter) {
            activateItem(m_selection);
            return;
        }
        if (changed) updateSelectionVisuals();
        return;
    }

    // ── 鼠标悬停高亮 ──
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(mouseMoved->position);
        for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
            if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                if (m_selection != i) {
                    m_selection = i;
                    updateSelectionVisuals();
                }
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

void MenuScene::activateItem(int index) {
    if (index == 0) {
        requestReplace(std::make_unique<PackScene>());
    } else if (index == 1) {
        // Settings — 待 Phase 4.3 实现
    } else if (index == 2) {
        requestPop();
    }
}

void MenuScene::updateSelectionVisuals() {
    for (int i = 0; i < (int)m_itemTexts.size(); ++i)
        m_itemTexts[i].setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
}

void MenuScene::update(float dt) {
    m_animTimer += dt;
}

void MenuScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;
    if (m_titleText.has_value()) target.draw(*m_titleText);
    for (auto& t : m_itemTexts) target.draw(t);
}
