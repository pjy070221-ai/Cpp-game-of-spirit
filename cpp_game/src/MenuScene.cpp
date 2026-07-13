#include "MenuScene.h"
#include "PackScene.h"
#include "SettingsScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <algorithm>
#include <cstdint>

MenuScene::MenuScene() : m_items({"Start Game", "Settings", "Exit"}) {
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    // ── 标题 "指尖振律" ──
    m_titleText.emplace(m_font, L"\u6307\u5C16\u632F\u5F8B", 72);
    m_titleText->setFillColor(sf::Color::White);
    auto tb = m_titleText->getLocalBounds();
    m_titleText->setOrigin({tb.size.x / 2.0f, 0.0f});
    m_titleText->setPosition({640.0f, 150.0f});

    // ── 浮动光效 ──
    std::mt19937 rng(12345);
    auto angD = std::uniform_real_distribution<float>(0, 6.2832f);
    auto rxD = std::uniform_real_distribution<float>(30, 200);
    auto ryD = std::uniform_real_distribution<float>(10, 80);
    auto spD = std::uniform_real_distribution<float>(0.2f, 0.8f);
    auto szD = std::uniform_real_distribution<float>(10, 70);
    auto cR = std::uniform_int_distribution<int>(100, 220);
    auto cG = std::uniform_int_distribution<int>(100, 220);
    auto cB = std::uniform_int_distribution<int>(180, 255);
    for (int i = 0; i < 8; ++i) {
        FloatLight fl;
        fl.angle = angD(rng);
        fl.orbitRadiusX = rxD(rng);
        fl.orbitRadiusY = ryD(rng);
        fl.speed = spD(rng);
        float rad = szD(rng);
        fl.shape = sf::CircleShape(rad);
        fl.shape.setFillColor(sf::Color((std::uint8_t)cR(rng), (std::uint8_t)cG(rng), (std::uint8_t)cB(rng), 35));
        fl.shape.setOrigin({rad, rad});
        m_floatLights.push_back(fl);
    }

    // ── 选项 ──
    for (int i = 0; i < (int)m_items.size(); ++i) {
        sf::Text txt(m_font, m_items[i], 36);
        txt.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
        auto ib = txt.getLocalBounds();
        txt.setOrigin({ib.size.x / 2.0f, 0.0f});
        txt.setPosition({640.0f, 330.0f + i * 80.0f});
        m_itemTexts.push_back(txt);
    }

    // ── 选项发光 ──
    m_optionGlow.setFillColor(sf::Color(180, 220, 255, 0));
    m_optionGlow2.setFillColor(sf::Color(100, 180, 255, 0));

    // ── 过渡遮罩 ──
    m_fadeOverlay.setSize({1280, 720});
    m_fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    m_flashOverlay.setSize({1280, 720});
    m_flashOverlay.setFillColor(sf::Color::Transparent);
}

void MenuScene::onEnter() {
    m_selection = 0;
    m_transitioning = false;
    m_fadeTimer = 0.0f;
    m_flashTimer = 0.0f;
    m_fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    updateSelectionVisuals();
}

sf::Color MenuScene::getNeonColor(float time) const {
    float t = time * 0.12f;
    auto r = (std::uint8_t)(170 + 85 * std::sin(t * 1.7f));
    auto g = (std::uint8_t)(140 + 115 * std::sin(t * 1.3f + 1.5f));
    auto b = (std::uint8_t)(200 + 55 * std::sin(t * 1.1f + 3.0f));
    return sf::Color(r, g, b);
}

void MenuScene::startTransition(int targetIndex) {
    m_pendingIndex = targetIndex;
    m_transitioning = true;
    m_fadeTimer = 0.0f;
    m_flashTimer = 0.0f; // 跳过白色闪光
}

void MenuScene::handleEvent(const sf::Event& event) {
    if (m_transitioning) return;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        bool changed = false;
        if (key->scancode == sf::Keyboard::Scan::Up || key->code == sf::Keyboard::Key::Up) {
            m_selection = (m_selection - 1 + (int)m_items.size()) % m_items.size(); changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Down || key->code == sf::Keyboard::Key::Down) {
            m_selection = (m_selection + 1) % m_items.size(); changed = true;
        } else if (key->scancode == sf::Keyboard::Scan::Enter || key->code == sf::Keyboard::Key::Enter) {
            if (m_selection == 2) { SceneManager::s_exitRequested = true; requestPop(); return; }
            SceneManager::s_transitionAlpha = 0.0f;
            startTransition(m_selection); return;
        }
        if (changed) updateSelectionVisuals();
        return;
    }
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
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(mouseBtn->position);
            for (int i = 0; i < (int)m_itemTexts.size(); ++i) {
                if (m_itemTexts[i].getGlobalBounds().contains(mousePos)) {
                    if (i == 2) { SceneManager::s_exitRequested = true; requestPop(); return; }
                    SceneManager::s_transitionAlpha = 0.0f;
                    startTransition(i); return;
                }
            }
        }
        return;
    }
}

void MenuScene::activateItem(int index) {
    if (index == 0) requestReplace(std::make_unique<PackScene>());
    else if (index == 1) requestPush(std::make_unique<SettingsScene>());
}

void MenuScene::updateSelectionVisuals() {
    for (int i = 0; i < (int)m_itemTexts.size(); ++i)
        m_itemTexts[i].setFillColor(i == m_selection ? sf::Color::Yellow : sf::Color(200, 200, 200));
}

void MenuScene::update(float dt) {
    m_animTimer += dt;

    // ── 霓虹标题 ──
    if (m_titleText.has_value()) m_titleText->setFillColor(getNeonColor(m_animTimer));

    // ── 共享过渡衰减 ──
    if (!m_transitioning && SceneManager::s_transitionAlpha > 0.001f)
        SceneManager::s_transitionAlpha = std::max(0.0f, SceneManager::s_transitionAlpha - dt * 2.5f);

    // ── 浮动光效 ──
    for (auto& fl : m_floatLights) {
        fl.angle += fl.speed * dt;
        fl.shape.setPosition({
            m_titleCenterX + std::cos(fl.angle) * fl.orbitRadiusX,
            m_titleCenterY + std::sin(fl.angle * 0.7f) * fl.orbitRadiusY
        });
        float pulse = 0.6f + 0.4f * std::sin(m_animTimer * 1.5f + fl.speed * 3.0f);
        auto c = fl.shape.getFillColor();
        c.a = (std::uint8_t)(pulse * 40);
        fl.shape.setFillColor(c);
    }

    // ── 过渡动画 ──
    if (m_transitioning) {
        if (m_flashTimer > 0.0f) {
            m_flashTimer -= dt;
            float fa = m_flashTimer / 0.08f;
            m_flashOverlay.setFillColor(sf::Color(255, 255, 255, (std::uint8_t)(fa * 220)));
        } else {
            m_fadeTimer += dt;
            float prog = std::min(1.0f, m_fadeTimer / 0.5f);
            SceneManager::s_transitionAlpha = prog;
            if (prog >= 1.0f) {
                m_transitioning = false;
                activateItem(m_pendingIndex);
            }
        }
    }
}

void MenuScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(10, 5, 20));
    if (!m_fontLoaded) return;

    // 浮动光效
    for (auto& fl : m_floatLights) target.draw(fl.shape);

    // 标题
    if (m_titleText.has_value()) target.draw(*m_titleText);

    // 选项发光
    if (!m_transitioning) {
        const auto& sel = m_itemTexts[m_selection];
        float pulse = 0.5f + 0.5f * std::sin(m_animTimer * 4.0f);
        float sx = sel.getPosition().x - sel.getLocalBounds().size.x / 2.0f - 15.0f;
        float sy = sel.getPosition().y - 8.0f;
        float sw = sel.getLocalBounds().size.x + 30.0f;
        float sh = sel.getLocalBounds().size.y + 16.0f;
        m_optionGlow2.setPosition({sx - 10, sy - 10});
        m_optionGlow2.setSize({sw + 20, sh + 20});
        m_optionGlow2.setFillColor(sf::Color(150, 210, 255, (std::uint8_t)(pulse * 30)));
        target.draw(m_optionGlow2);
        m_optionGlow.setPosition({sx, sy});
        m_optionGlow.setSize({sw, sh});
        m_optionGlow.setFillColor(sf::Color(200, 235, 255, (std::uint8_t)(pulse * 50)));
        target.draw(m_optionGlow);
        sf::RectangleShape border({sw, sh});
        border.setPosition({sx, sy});
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(1.5f);
        border.setOutlineColor(sf::Color(200, 235, 255, (std::uint8_t)(pulse * 120)));
        target.draw(border);
    }

    // 选项文字
    for (auto& t : m_itemTexts) target.draw(t);

    // 点击闪光 + 渐黑遮罩
    if (m_flashTimer > 0.0f) target.draw(m_flashOverlay);
    if (m_fadeOverlay.getFillColor().a > 0) target.draw(m_fadeOverlay);

    // 共享过渡遮罩（来自 TitleScene）
    if (SceneManager::s_transitionAlpha > 0.001f && !m_transitioning) {
        sf::RectangleShape ov({1280, 720});
        ov.setFillColor(sf::Color(0, 0, 0, (std::uint8_t)(SceneManager::s_transitionAlpha * 255)));
        target.draw(ov);
    }
}


