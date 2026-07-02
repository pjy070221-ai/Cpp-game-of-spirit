#pragma once
#include "IScene.h"
#include "../core/ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <string>

// ╔══════════════════════════════════════════════════════════╗
// ║  ResultScene — 结算画面                                    ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】展示演奏结果：分数滚动、连击、评级。                ║
// ║    接收 GameplayScene 传来的 GameResult 数据。              ║
// ║                                                          ║
// ║  【与其他 Scene 的关系】                                    ║
// ║    GameplayScene → 自动 → ResultScene                    ║
// ║    ResultScene → ENTER/ESC → PackScene                   ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝

struct GameResult {
    int score = 0;
    int maxCombo = 0;
    std::string songTitle;
};

class ResultScene : public IScene
{
public:
    explicit ResultScene(const GameResult& result);
    ~ResultScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    GameResult m_result;

    // 动画计时
    float m_elapsed = 0.f;
    float m_displayScore = 0.f;   // 滚动显示的分数

    // 评级
    char m_grade = 'C';
    sf::Color gradeColor() const;

    // UI
    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_songText;
    sf::Text m_scoreText;
    sf::Text m_comboText;
    sf::Text m_gradeText;
    sf::Text m_hintText;

    // 背景
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    ParticleSystem m_stars;

    void buildBackground();
};
