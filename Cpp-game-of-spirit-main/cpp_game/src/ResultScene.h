#pragma once

#include "GameplayScene.h"
#include "ParticleSystem.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

class ResultScene : public IScene {
public:
    ResultScene(const ResultData& data, const std::string& grade);
    ~ResultScene() override = default;
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
private:
    sf::Color gradeColor() const;


    ResultData m_data;
    std::string m_grade;
    sf::Font m_font;
    bool m_fontLoaded = false;
    float m_timer = 0.0f;

    // Animated texts
    std::optional<sf::Text> m_titleText, m_scoreText, m_gradeText;
    std::optional<sf::Text> m_perfectText, m_greatText, m_goodText, m_missText, m_comboText;
    std::optional<sf::Text> m_retryText, m_continueText;
    std::optional<sf::Text> m_celebrationText;

    // Fireworks
    ParticleSystem m_fireworks;
    int m_lastBurst = 0;
};

