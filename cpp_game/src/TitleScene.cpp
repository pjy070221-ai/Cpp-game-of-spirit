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

TitleScene::TitleScene() {
    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (!fontPtr) return;
    m_font = *fontPtr;
    m_fontLoaded = true;

    if (m_textLayer.resize({1280, 720})) {
        m_textLayer.clear(sf::Color::Transparent);
        sf::Text t(m_font, L"    ", 80);
        t.setFillColor(sf::Color::White);
        m_textLayer.draw(t);
        m_textLayer.display();
    }

    // loading bar
    m_loadTrack.setSize({400.0f, 6.0f});
    m_loadTrack.setFillColor(sf::Color(60, 40, 80, 150));
    m_loadTrack.setPosition({440.0f, 500.0f});
    m_loadFill.setFillColor(sf::Color(0, 200, 255));
    m_loadFill.setSize({0.0f, 6.0f});
    m_loadFill.setPosition({440.0f, 500.0f});

    // title chars: 指尖振律
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

    // subtitle: Cross Beat
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

    // prompt: 按任意键继续
    m_promptText.emplace(m_font);
    m_promptText->setString(L"\u6309\u4EFB\u610F\u952E\u7EE7\u7EED");
    m_promptText->setCharacterSize(24);
    m_promptText->setFillColor(sf::Color(200, 200, 200, 0));
    auto pb = m_promptText->getLocalBounds();
    m_promptText->setOrigin({pb.size.x / 2.0f, 0.0f});
    m_promptText->setPosition({640.0f, 520.0f});

    // background gradient
    m_bgGradient.resize(4);
    m_bgGradient[0] = sf::Vertex({0.0f, 0.0f}, sf::Color(8, 4, 16));
    m_bgGradient[1] = sf::Vertex({1280.0f, 0.0f}, sf::Color(8, 4, 16));
    m_bgGradient[2] = sf::Vertex({0.0f, 720.0f}, sf::Color(20, 8, 40));
    m_bgGradient[3] = sf::Vertex({1280.0f, 720.0f}, sf::Color(20, 8, 40));
}

void TitleScene::onEnter() {
    m_state = State::Loading;
    m_elapsedTime = 0.0f;
    m_loadProgress = 0.0f;
    m_globalAlpha = 0.0f;
    m_subAlpha = 0.0f;
    m_promptReady = false;
    m_titleNotes.clear();
    m_titleRings.clear();
    m_noteSpawnTimer = 0.0f;
    m_ringSpawnTimer = 0.0f;
    m_promptText->setFillColor(sf::Color(200, 200, 200, 0));
    triggerGlitch();
}

void TitleScene::onExit() {}

void TitleScene::handleEvent(const sf::Event& event) {
    if (m_state != State::Idle) return;
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return;
    if (!(key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape)) {
        m_transitioning = true;
        m_transitionTimer = 0.0f;
    }
}

void TitleScene::triggerGlitch() {
    static std::mt19937 rng(std::random_device{}());
    m_glitchTimer = 0.0f;
    m_glitchDuration = 0.05f + std::uniform_real_distribution<float>(0, 0.1f)(rng);
    m_glitchIntensity = std::uniform_real_distribution<float>(0.2f, 0.8f)(rng);
}

void TitleScene::spawnTitleNote() {
    if (m_titleNotes.size() >= 6) return;
    static std::mt19937 rng(std::random_device{}());
    static const sf::Color colors[4] = {
        sf::Color(0, 220, 255),
        sf::Color(255, 100, 200),
        sf::Color(255, 210, 0),
        sf::Color(100, 230, 100)
    };
    static const sf::Color outlines[4] = {
        sf::Color(0, 180, 220),
        sf::Color(200, 60, 160),
        sf::Color(200, 170, 0),
        sf::Color(60, 190, 60)
    };

    // spawn from random window edge, horizontal or vertical
    int edge = std::uniform_int_distribution<int>(0, 3)(rng);
    float x = 0.0f, y = 0.0f, vx = 0.0f, vy = 0.0f;
    float speed = std::uniform_real_distribution<float>(250.0f, 400.0f)(rng);
    float drift = std::uniform_real_distribution<float>(-40.0f, 40.0f)(rng);
    switch (edge) {
    case 0: x = std::uniform_real_distribution<float>(80.0f, 1200.0f)(rng); y = -50.0f; vx = drift; vy = speed; break;
    case 1: x = std::uniform_real_distribution<float>(80.0f, 1200.0f)(rng); y = 770.0f; vx = drift; vy = -speed; break;
    case 2: x = -50.0f; y = std::uniform_real_distribution<float>(80.0f, 640.0f)(rng); vx = speed; vy = drift; break;
    default: x = 1330.0f; y = std::uniform_real_distribution<float>(80.0f, 640.0f)(rng); vx = -speed; vy = drift; break;
    }

    int ci = std::uniform_int_distribution<int>(0, 3)(rng);
    TitleNote n;
    n.shape.setSize({60.0f, 24.0f});
    n.shape.setFillColor(colors[ci]);
    n.shape.setOutlineThickness(2.0f);
    n.shape.setOutlineColor(outlines[ci]);
    n.shape.setOrigin({30.0f, 12.0f});
    n.shape.setPosition({x, y});
    n.velocity = {vx, vy};
    n.life = 8.0f;
    n.maxLife = 8.0f;
    m_titleNotes.push_back(n);
}

void TitleScene::spawnTitleRing() {
    static std::mt19937 rng(std::random_device{}());
    static const sf::Color colors[4] = {
        sf::Color(0, 220, 255),
        sf::Color(255, 100, 200),
        sf::Color(255, 210, 0),
        sf::Color(100, 230, 100)
    };
    int ci = std::uniform_int_distribution<int>(0, 3)(rng);
    const auto& c = colors[ci];

    TitleRing r;
    r.shape.setRadius(6.0f);
    r.shape.setPosition({
        640.0f + std::uniform_real_distribution<float>(-250.0f, 250.0f)(rng),
        300.0f + std::uniform_real_distribution<float>(-180.0f, 180.0f)(rng)
    });
    r.shape.setFillColor(sf::Color::Transparent);
    r.shape.setOutlineThickness(3.0f);
    r.shape.setOutlineColor(sf::Color(c.r, c.g, c.b, 200));
    r.color = c;
    r.life = 2.0f;
    r.maxLife = 2.0f;
    m_titleRings.push_back(r);
}

void TitleScene::update(float dt) {
    m_elapsedTime += dt;

    sf::FloatRect screen({0, 0}, {1280, 720});
    m_stars.spawnStars(dt, screen, 60);
    m_dust.spawnStars(dt, screen, 20);
    m_stars.update(dt);
    m_dust.update(dt);

    if (m_state != State::Loading) {
        static std::mt19937 rng(std::random_device{}());
        if (m_glitchTimer >= m_glitchDuration) {
            if (std::uniform_real_distribution<float>(0, 1)(rng) < 0.005f) triggerGlitch();
        } else {
            m_glitchTimer += dt;
        }
    }

    // Loading
    if (m_state == State::Loading) {
        m_loadProgress = std::min(1.0f, m_elapsedTime / LOAD_DURATION);
        m_loadFill.setSize({m_loadProgress * 400.0f, 6.0f});
        if (m_loadProgress >= 1.0f) { m_state = State::TitleReveal; m_elapsedTime = 0.0f; }
        return;
    }

    // TitleReveal
    if (m_state == State::TitleReveal) {
        float rt = m_elapsedTime;
        m_globalAlpha = std::min(1.0f, rt / 0.8f);
        for (auto& cd : m_titleChars) {
            float lt = std::max(0.0f, (rt - cd.animDelay) / 0.6f);
            float p = easeOutBack(std::min(1.0f, lt));
            cd.text.setPosition({cd.targetX, cd.targetY + 200.0f * (1.0f - p)});
            cd.text.setScale({0.5f + 0.5f * p, 0.5f + 0.5f * p});
            sf::Color c = sf::Color::White; c.a = (uint8_t)(std::min(1.0f, lt * 2.0f) * 255); cd.text.setFillColor(c);
        }
        for (auto& cd : m_subChars) {
            float lt = std::max(0.0f, (rt - cd.animDelay) / 0.5f);
            float p = easeOutCubic(std::min(1.0f, lt));
            cd.text.setPosition({cd.targetX, cd.targetY + 60.0f * (1.0f - p)});
            m_subAlpha = std::min(1.0f, lt * 2.0f);
            cd.text.setFillColor(sf::Color(0, 200, 255, (uint8_t)(m_subAlpha * 255)));
        }
        if (rt > 2.0f) { m_state = State::Idle; m_promptReady = true; }
    }

    // notes and rings - active during TitleReveal and Idle
    if (m_state == State::TitleReveal || m_state == State::Idle) {
        m_noteSpawnTimer += dt;
        if (m_noteSpawnTimer >= 0.6f) { m_noteSpawnTimer = 0.0f; spawnTitleNote(); }
        m_ringSpawnTimer += dt;
        if (m_ringSpawnTimer >= 0.4f) { m_ringSpawnTimer = 0.0f; spawnTitleRing(); }

        // update notes: move + life + remove
        for (auto& n : m_titleNotes) {
            n.shape.move(n.velocity * dt);
            n.life -= dt;
        }
        m_titleNotes.erase(std::remove_if(m_titleNotes.begin(), m_titleNotes.end(),
            [](auto& n) { return n.life <= 0 || n.shape.getPosition().x < -100 || n.shape.getPosition().x > 1380 || n.shape.getPosition().y < -100 || n.shape.getPosition().y > 820; }), m_titleNotes.end());

        // update rings: expand + fade
        for (auto& r : m_titleRings) {
            r.life -= dt;
            float t = 1.0f - r.life / r.maxLife;
            r.shape.setRadius(6.0f * (1.0f + t * 8.0f));
            sf::Color oc = r.color; oc.a = (uint8_t)((1.0f - t) * 200);
            r.shape.setOutlineColor(oc);
        }
        m_titleRings.erase(std::remove_if(m_titleRings.begin(), m_titleRings.end(),
            [](auto& r) { return r.life <= 0; }), m_titleRings.end());
    }

    // transition
    if (m_transitioning) {
        m_transitionTimer += dt;
        float prog = std::min(1.0f, m_transitionTimer / 0.5f);
        SceneManager::s_transitionAlpha = prog;
        if (prog >= 1.0f) { m_transitioning = false; requestReplace(std::make_unique<MenuScene>()); }
        return;
    }

    // Idle: blink prompt
    if (m_state == State::Idle && m_promptReady) {
        float blink = (std::sin(m_elapsedTime * 3.0f) + 1.0f) * 0.5f;
        m_promptText->setFillColor(sf::Color(200, 200, 200, (uint8_t)(blink * 200)));
    }
}

void TitleScene::render(sf::RenderTarget& target) {
    target.clear(sf::Color(8, 4, 16));
    target.draw(m_bgGradient);
    m_stars.render(target);
    m_dust.render(target);

    // flying notes from edges (with 6-layer gradient trail)
    for (auto& n : m_titleNotes) {
        auto pos = n.shape.getPosition();
        auto col = n.shape.getFillColor();
        sf::Vector2f velDir = n.velocity;
        float len = std::sqrt(velDir.x * velDir.x + velDir.y * velDir.y);
        if (len > 0.001f) velDir /= len;

        // 6-layer gradient trail: alpha fade, scale shrink, color darken
        float offsets[6] = {6.0f, 14.0f, 24.0f, 36.0f, 50.0f, 66.0f};
        int alphas[6] = {70, 48, 30, 18, 9, 3};
        float scales[6] = {1.00f, 0.94f, 0.88f, 0.82f, 0.76f, 0.70f};
        sf::Color fadeCol[6] = {
            col,
            sf::Color((int)(col.r*0.85f),(int)(col.g*0.85f),(int)(col.b*0.85f)),
            sf::Color((int)(col.r*0.70f),(int)(col.g*0.70f),(int)(col.b*0.70f)),
            sf::Color((int)(col.r*0.55f),(int)(col.g*0.55f),(int)(col.b*0.55f)),
            sf::Color((int)(col.r*0.40f),(int)(col.g*0.40f),(int)(col.b*0.40f)),
            sf::Color((int)(col.r*0.25f),(int)(col.g*0.25f),(int)(col.b*0.25f))
        };
        for (int t = 0; t < 6; ++t) {
            sf::RectangleShape ghost = n.shape;
            ghost.setScale({scales[t], scales[t]});
            ghost.setPosition({pos.x - velDir.x * offsets[t], pos.y - velDir.y * offsets[t]});
            ghost.setFillColor(sf::Color(fadeCol[t].r, fadeCol[t].g, fadeCol[t].b, (uint8_t)alphas[t]));
            target.draw(ghost);
        }
        // draw the note itself
        target.draw(n.shape);
    }

    // floating rings
    for (auto& r : m_titleRings) { target.draw(r.shape); }

    // glitch offset
    float gx = 0.0f, gy = 0.0f;
    if (m_glitchTimer < m_glitchDuration) {
        static std::mt19937 rng(std::random_device{}());
        gx = std::uniform_real_distribution<float>(-5.0f, 5.0f)(rng) * m_glitchIntensity;
        gy = std::uniform_real_distribution<float>(-3.0f, 3.0f)(rng) * m_glitchIntensity;
    }

    if (m_fontLoaded && m_state != State::Loading) {
        for (auto& cd : m_titleChars) {
            if (cd.text.getFillColor().a > 0) {
                cd.text.move({gx, gy}); target.draw(cd.text); cd.text.move({-gx, -gy});
            }
        }
        for (auto& cd : m_subChars) { target.draw(cd.text); }
        if (m_promptReady) target.draw(*m_promptText);
    }

    // loading bar
    if (m_state == State::Loading) { target.draw(m_loadTrack); target.draw(m_loadFill); }
}

