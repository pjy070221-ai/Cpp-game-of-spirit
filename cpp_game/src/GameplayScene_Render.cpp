#include "GameplayScene.h"
#include <cmath>

float GameplayScene::getTrackCenterX(int track) const {
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth
                   + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;
    return startX + track * (m_trackWidth + m_trackSpacing) + m_trackWidth / 2.0f;
}

void GameplayScene::render(sf::RenderTarget& target) {
    // background with subtle pulse glow
    float glow = m_glowIntensity * 30.0f;
    sf::Color topColor((int)(10 + glow * 0.3f), (int)(5 + glow * 0.2f), (int)(20 + glow * 0.5f));
    sf::Color botColor((int)(20 + glow * 0.5f), (int)(10 + glow * 0.3f), (int)(40 + glow));
    m_bgGradient[0].color = topColor;
    m_bgGradient[1].color = topColor;
    m_bgGradient[2].color = botColor;
    m_bgGradient[3].color = botColor;
    target.draw(m_bgGradient);

    // track columns with glow when notes approach
    for (int t = 0; t < m_trackCount; ++t) {
        float trackGlow = 0.0f;
        for (auto& nr : m_noteRuntimes) {
            if (!nr.active || nr.processed || nr.track != t) continue;
            float dist = std::abs(nr.y - m_judgmentLineY);
            if (dist < 200.0f) trackGlow = std::max(trackGlow, 1.0f - dist / 200.0f);
        }
        float g = 60.0f + trackGlow * 120.0f;
        m_tracks[t].setFillColor(sf::Color((int)(30 + g * 0.3f), (int)(20 + g * 0.5f),
                                            (int)(50 + g * 0.8f), (int)(80 + (int)(trackGlow * 80))));
        target.draw(m_tracks[t]);
    }

    // note circles
    for (auto& n : m_activeShapes)
        target.draw(n);

    // hit particles
    m_hitFX.render(target);

    // judgment line with pulse
    float pulseAlpha = 128 + (int)(127 * m_glowIntensity);
    m_judgmentLineShape.setFillColor(sf::Color(0, 255, 255, pulseAlpha));
    m_judgmentLineShape.setSize({m_screenWidth, 3.0f + m_glowIntensity});
    target.draw(m_judgmentLineShape);

    // HUD
    if (m_scoreText.has_value()) {
        m_scoreText->setPosition({20.0f, 10.0f});
        target.draw(*m_scoreText);
    }
    if (m_comboText.has_value() && m_combo > 0) {
        m_comboText->setPosition({640.0f, 600.0f});
        m_comboText->setOrigin({m_comboText->getLocalBounds().size.x / 2.0f, 0.0f});
        target.draw(*m_comboText);
    }
    if (m_judgmentText.has_value() && m_judgmentDisplayTimer > 0) {
        float scale = 1.0f + (0.8f - m_judgmentDisplayTimer) * 0.5f;
        m_judgmentText->setScale({scale, scale});
        m_judgmentText->setPosition({640.0f, 300.0f});
        m_judgmentText->setOrigin({m_judgmentText->getLocalBounds().size.x / 2.0f, 0.0f});
        target.draw(*m_judgmentText);
    }
    if (m_songTitleText.has_value()) {
        m_songTitleText->setPosition({640.0f, 680.0f});
        m_songTitleText->setOrigin({m_songTitleText->getLocalBounds().size.x / 2.0f, 0.0f});
        target.draw(*m_songTitleText);
    }
}

