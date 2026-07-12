#pragma once

#include "IScene.h"
#include "ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class TitleScene : public IScene {
public:
    TitleScene();
    ~TitleScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    // ── 状态机 ──
    enum class State { Loading, Shattering, TitleReveal, Idle };
    State m_state = State::Loading;
    float m_elapsedTime = 0.0f;

    // ── Loading ──
    float m_loadProgress = 0.0f;
    static constexpr float LOAD_DURATION = 2.5f;
    sf::RectangleShape m_loadTrack, m_loadFill;

    // ── Shattering ──
    float m_shatterTimer = 0.0f;
    static constexpr float SHATTER_DURATION = 0.8f;
    struct ShatterCrack { float angle, maxLen, delay; };
    std::vector<ShatterCrack> m_shatterCrackData;
    sf::VertexArray m_shatterCrackGeom{ sf::PrimitiveType::Lines };

    // ── 背景分层 ──
    sf::VertexArray m_bgShards{ sf::PrimitiveType::Triangles };
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    sf::VertexArray m_bgEnergy{ sf::PrimitiveType::TriangleFan };
    sf::VertexArray m_bgCracks{ sf::PrimitiveType::Lines };
    sf::VertexArray m_bgFlash{ sf::PrimitiveType::Triangles };
    ParticleSystem m_stars, m_dust;
    sf::RenderTexture m_textLayer;

    // ── 标题字符动画 ──
    struct CharData {
        sf::Text text;
        float baseSize = 80.0f;
        float animDelay = 0.0f;
        float targetX = 0.0f, targetY = 0.0f;
        CharData(const sf::Font& f) : text(f) {}
    };
    std::vector<CharData> m_titleChars, m_subChars;
    sf::Font m_font;
    bool m_fontLoaded = false;

    // ── Glitch ──
    float m_glitchTimer = 0.0f, m_glitchDuration = 0.0f, m_glitchIntensity = 0.0f;
    void triggerGlitch();
    float m_globalAlpha = 0.0f, m_subAlpha = 0.0f;

    // ── 提示文字 ──
    std::optional<sf::Text> m_promptText;
    bool m_promptReady = false;

    // ── 过渡 → MenuScene ──
    bool m_transitioning = false;
    float m_transitionTimer = 0.0f;
};


