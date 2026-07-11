#include "TitleScene.h"
#include "MenuScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Easing.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <random>
#include <cstdint>
#include <algorithm>

// ── 构造函数 ──
TitleScene::TitleScene() {
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    // 预渲染文字层
    if (m_textLayer.resize({1280, 720})) {
        m_textLayer.clear(sf::Color::Transparent);
        sf::Text t(m_font, L"    ", 80);
        t.setFillColor(sf::Color::White);
        m_textLayer.draw(t);
        m_textLayer.display();
    }

    // Loading 条
    m_loadTrack.setSize({400.0f, 6.0f});
    m_loadTrack.setFillColor(sf::Color(60, 40, 80, 150));
    m_loadTrack.setPosition({440.0f, 500.0f});
    m_loadFill.setFillColor(sf::Color(0, 200, 255));
    m_loadFill.setSize({0.0f, 6.0f});
    m_loadFill.setPosition({440.0f, 500.0f});

    // 标题字符 "指尖旋律" —— 每个字单独 sf::Text
    const wchar_t title[] = L"\u6307\u5C16\u632F\u5F8B";
    float startX = 640.0f - 4 * 40.0f;
    for (int i = 0; i < 4; ++i) {
        CharData cd(m_font);
        cd.text.setString(std::wstring(1, title[i]));
        cd.text.setCharacterSize(80);
        cd.text.setFillColor(sf::Color::White);
        cd.targetX = startX + i * 80.0f + 40.0f;
        cd.targetY = 300.0f;
        cd.animDelay = i * 0.15f;
        auto tb = cd.text.getLocalBounds();
        cd.text.setOrigin({tb.size.x / 2.0f, tb.size.y / 2.0f});
        cd.text.setPosition({cd.targetX, cd.targetY + 200.0f});
        m_titleChars.push_back(cd);
    }

    // 副标题 "Cross Beat"
    {
        CharData cd(m_font);
        cd.text.setString(L"Cross Beat");
        cd.text.setCharacterSize(28);
        cd.text.setFillColor(sf::Color(0, 200, 255, 0));
        cd.targetX = 640.0f;
        cd.targetY = 390.0f;
        cd.animDelay = 0.6f;
        auto tb = cd.text.getLocalBounds();
        cd.text.setOrigin({tb.size.x / 2.0f, tb.size.y / 2.0f});
        cd.text.setPosition({cd.targetX, cd.targetY + 60.0f});
        m_subChars.push_back(cd);
    }

    // 提示文字
    m_promptText.emplace(m_font); m_promptText->setString(L">> Press Any Key <<");
    m_promptText->setCharacterSize(24);
    m_promptText->setFillColor(sf::Color(200, 200, 200, 0));
    auto pb = m_promptText->getLocalBounds();
    m_promptText->setOrigin({pb.size.x / 2.0f, 0.0f});
    m_promptText->setPosition({640.0f, 520.0f});

    // 背景渐变
    m_bgGradient.resize(4);
    m_bgGradient[0] = sf::Vertex({0.0f, 0.0f}, sf::Color(8, 4, 16));
    m_bgGradient[1] = sf::Vertex({1280.0f, 0.0f}, sf::Color(8, 4, 16));
    m_bgGradient[2] = sf::Vertex({0.0f, 720.0f}, sf::Color(20, 8, 40));
    m_bgGradient[3] = sf::Vertex({1280.0f, 720.0f}, sf::Color(20, 8, 40));

    // 裂纹数据
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> angleDist(-0.8f, 0.8f);
    std::uniform_real_distribution<float> lenDist(100.0f, 350.0f);
    std::uniform_real_distribution<float> delayDist(0.0f, 0.4f);
    for (int i = 0; i < 20; ++i) {
        m_shatterCrackData.push_back({
            angleDist(rng), lenDist(rng), delayDist(rng)
        });
    }
    m_shatterCrackGeom.resize(m_shatterCrackData.size() * 2);
}

// ── 生命周期 ──
void TitleScene::onEnter() {
    m_state = State::Loading;
    m_elapsedTime = 0.0f;
    m_loadProgress = 0.0f;
    m_shatterTimer = 0.0f;
    m_globalAlpha = 0.0f;
    m_subAlpha = 0.0f;
    m_promptReady = false;
    m_promptText->setFillColor(sf::Color(200, 200, 200, 0));
    triggerGlitch();
}

void TitleScene::onExit() {}

void TitleScene::handleEvent(const sf::Event& event) {
    if (m_state != State::Idle) return;
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return;
    // 按任意键（ESC 除外）进入主菜单
    if (!(key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape))
        requestReplace(std::make_unique<MenuScene>());
}

void TitleScene::triggerGlitch() {
    static std::mt19937 rng(std::random_device{}());
    m_glitchTimer = 0.0f;
    m_glitchDuration = 0.05f + std::uniform_real_distribution<float>(0, 0.1f)(rng);
    m_glitchIntensity = std::uniform_real_distribution<float>(0.2f, 0.8f)(rng);
}

void TitleScene::update(float dt) {
    m_elapsedTime += dt;

    // 星空 + 浮尘
    sf::FloatRect screen({0, 0}, {1280, 720});
    m_stars.spawnStars(dt, screen, 60);
    m_dust.spawnStars(dt, screen, 20);
    m_stars.update(dt);
    m_dust.update(dt);

    // ── Glitch ──
    if (m_state != State::Loading && m_state != State::Shattering) {
        static std::mt19937 rng(std::random_device{}());
        if (m_glitchTimer >= m_glitchDuration) {
            if (std::uniform_real_distribution<float>(0, 1)(rng) < 0.005f) triggerGlitch();
        } else {
            m_glitchTimer += dt;
        }
    }

    // ── 加载阶段 ──
    if (m_state == State::Loading) {
        m_loadProgress = std::min(1.0f, m_elapsedTime / LOAD_DURATION);
        m_loadFill.setSize({m_loadProgress * 400.0f, 6.0f});
        if (m_loadProgress >= 1.0f) {
            m_state = State::Shattering;
            m_shatterTimer = 0.0f;
        }
        return;
    }

    // ── 碎裂阶段 ──
    if (m_state == State::Shattering) {
        m_shatterTimer += dt;
        float t = std::min(1.0f, m_shatterTimer / SHATTER_DURATION);
        // 更新裂纹几何
        for (size_t i = 0; i < m_shatterCrackData.size(); ++i) {
            const auto& c = m_shatterCrackData[i];
            float alpha = std::clamp((t - c.delay) / 0.2f, 0.0f, 1.0f);
            float len = c.maxLen * easeOutExpo(alpha);
            float ex = std::sin(c.angle) * len;
            float ey = -std::cos(c.angle) * len;
            float cx = 640.0f, cy = 360.0f;
            m_shatterCrackGeom[i * 2] = sf::Vertex({cx, cy}, sf::Color(0, 200, 255, (int)(alpha * 150)));
            m_shatterCrackGeom[i * 2 + 1] = sf::Vertex({cx + ex, cy + ey}, sf::Color(0, 200, 255, (int)(alpha * 80)));
        }
        if (t >= 1.0f) {
            m_state = State::TitleReveal;
            m_elapsedTime = 0.0f;
        }
        return;
    }

    // ── 标题展示阶段 ──
    if (m_state == State::TitleReveal) {
        float revealTime = m_elapsedTime;
        // 全局淡入
        m_globalAlpha = std::min(1.0f, revealTime / 0.8f);

        // 逐字飞入
        for (auto& cd : m_titleChars) {
            float localT = std::max(0.0f, (revealTime - cd.animDelay) / 0.6f);
            float progress = easeOutBack(std::min(1.0f, localT));
            float y = cd.targetY + 200.0f * (1.0f - progress);
            float scale = 0.5f + 0.5f * progress;
            cd.text.setPosition({cd.targetX, y});
            cd.text.setScale({scale, scale});
            sf::Color c = sf::Color::White;
            c.a = (std::uint8_t)(std::min(1.0f, localT * 2.0f) * 255);
            cd.text.setFillColor(c);
        }

        // 副标题
        for (auto& cd : m_subChars) {
            float localT = std::max(0.0f, (revealTime - cd.animDelay) / 0.5f);
            float progress = easeOutCubic(std::min(1.0f, localT));
            cd.text.setPosition({cd.targetX, cd.targetY + 60.0f * (1.0f - progress)});
            m_subAlpha = std::min(1.0f, localT * 2.0f);
            sf::Color c(0, 200, 255, (std::uint8_t)(m_subAlpha * 255));
            cd.text.setFillColor(c);
        }

        if (revealTime > 2.0f) {
            m_state = State::Idle;
            m_promptReady = true;
        }
        return;
    }

    // ── Idle ──
    if (m_state == State::Idle && m_promptReady) {
        float blink = (std::sin(m_elapsedTime * 3.0f) + 1.0f) * 0.5f;
        m_promptText->setFillColor(sf::Color(200, 200, 200, (std::uint8_t)(blink * 200)));
    }
}

void TitleScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(8, 4, 16));

    // 背景渐变
    target.draw(m_bgGradient);

    // 星空 + 浮尘
    m_stars.render(target);
    m_dust.render(target);

    // 裂纹（Shattering 阶段）
    if (m_state == State::Shattering || m_state == State::TitleReveal) {
        target.draw(m_shatterCrackGeom);
    }

    // Glitch 偏移
    float glitchX = 0.0f, glitchY = 0.0f;
    if (m_glitchTimer < m_glitchDuration) {
        static std::mt19937 rng(std::random_device{}());
        glitchX = std::uniform_real_distribution<float>(-5.0f, 5.0f)(rng) * m_glitchIntensity;
        glitchY = std::uniform_real_distribution<float>(-3.0f, 3.0f)(rng) * m_glitchIntensity;
    }

    if (m_fontLoaded && m_state != State::Loading) {
        // 标题字符（带 Glitch 偏移）
        for (auto& cd : m_titleChars) {
            if (cd.text.getFillColor().a > 0) {
                cd.text.move({glitchX, glitchY});
                target.draw(cd.text);
                cd.text.move({-glitchX, -glitchY});
            }
        }
        // 副标题
        for (auto& cd : m_subChars) {
            target.draw(cd.text);
        }
        // 提示文字
        if (m_promptReady) target.draw(*m_promptText);
    }

    // Loading 阶段绘制进度条
    if (m_state == State::Loading) {
        target.draw(m_loadTrack);
        target.draw(m_loadFill);
    }
}




