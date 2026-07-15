#pragma once

#include "IScene.h"
#include "ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

struct TitleNote {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float life = 0.0f, maxLife = 1.0f;
};

struct TitleRing {
    sf::CircleShape shape;
    sf::Color color;
    float life = 0.0f, maxLife = 2.0f;
};

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
    // state machine
    enum class State { Loading, TitleReveal, Idle };
    State m_state = State::Loading;
    float m_elapsedTime = 0.0f;

    // Loading
    float m_loadProgress = 0.0f;
    static constexpr float LOAD_DURATION = 2.5f;
    sf::RectangleShape m_loadTrack, m_loadFill;

    // background layers
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    ParticleSystem m_stars, m_dust;
    sf::RenderTexture m_textLayer;

    // title character animation
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

    // glitch effect
    float m_glitchTimer = 0.0f, m_glitchDuration = 0.0f, m_glitchIntensity = 0.0f;
    void triggerGlitch();
    float m_globalAlpha = 0.0f, m_subAlpha = 0.0f;

    // prompt text
    std::optional<sf::Text> m_promptText;
    bool m_promptReady = false;

    // flying notes from edges
    std::vector<TitleNote> m_titleNotes;
    float m_noteSpawnTimer = 0.0f;
    void spawnTitleNote();

    // floating rings around title
    std::vector<TitleRing> m_titleRings;
    float m_ringSpawnTimer = 0.0f;
    void spawnTitleRing();

    // transition to MenuScene
    bool m_transitioning = false;
    float m_transitionTimer = 0.0f;
    sf::Texture m_bgTexture;
    std::unique_ptr<sf::Sprite> m_bgSprite;
};

