#include "GameplayScene.h"
#include <cmath>

float GameplayScene::getTrackCenterX(int track) const {
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth
                   + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;
    return startX + track * (m_trackWidth + m_trackSpacing) + m_trackWidth / 2.0f;
}

void GameplayScene::render(sf::RenderTarget& target) {
    // ── 屏幕震动 ──
    sf::View originalView = target.getView();
    if (m_anomalySystem.isActive(AnomalyType::ScreenShake)) {
        float intensity = m_anomalySystem.getIntensity(AnomalyType::ScreenShake);
        static std::mt19937 rng(std::random_device{}());
        float ms = 8.0f * intensity;
        float ox = std::uniform_real_distribution<float>(-ms, ms)(rng);
        float oy = std::uniform_real_distribution<float>(-ms, ms)(rng);
        sf::View shaken = originalView;
        shaken.move({ox, oy});
        target.setView(shaken);
    }
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

        // 节奏大师 轨道底部发光区
        float glowA = 15.0f + trackGlow * 60.0f;
        sf::RectangleShape gz({m_trackWidth, 80.0f});
        gz.setPosition({m_tracks[t].getPosition().x, m_judgmentLineY - 80.0f});
        gz.setFillColor(sf::Color(0, 200, 255, (int)glowA));
        target.draw(gz);
    }

    // 节奏大师 矩形音符（跳过已判定的）
    for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (m_noteRuntimes[i].active && !m_noteRuntimes[i].processed)
            target.draw(m_activeShapes[i]);
    }

    // ── 节奏大师 HP 血条（左栏）──
    {
        float hpRatio = (float)m_hp / m_maxHp;
        sf::RectangleShape bg({22.0f, 260.0f});
        bg.setPosition({12.0f, 230.0f});
        bg.setFillColor(sf::Color(40, 15, 20, 180));
        bg.setOutlineThickness(1.0f);
        bg.setOutlineColor(sf::Color(80, 30, 40, 200));
        target.draw(bg);

        sf::RectangleShape fill({18.0f, 254.0f * std::max(0.0f, hpRatio)});
        fill.setPosition({14.0f, 232.0f + 254.0f * (1.0f - std::max(0.0f, hpRatio))});
        if      (hpRatio > 0.5f) fill.setFillColor(sf::Color(0, 220, 80));
        else if (hpRatio > 0.25f) fill.setFillColor(sf::Color(240, 220, 0));
        else                      fill.setFillColor(sf::Color(240, 30, 30));
        target.draw(fill);
    }

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
        // 节奏大师: Combo 缩放
        float cs = 1.0f;
        if      (m_combo >= 50) cs = 2.0f;
        else if (m_combo >= 30) cs = 1.6f;
        else if (m_combo >= 10) cs = 1.2f;
        m_comboText->setScale({cs, cs});
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

    // 节奏大师 歌曲进度条
    {
        float duration = 30.0f;
        if (!m_noteData.empty())
            duration = m_noteData.back().time + 2.0f;
        float progress = std::min(1.0f, m_simTime / duration);
        sf::RectangleShape pb({1260.0f * progress, 4.0f});
        pb.setPosition({10.0f, 716.0f});
        pb.setFillColor(sf::Color(0, 200, 255, 180));
        target.draw(pb);
    }
    // ── 恢复视图 + 闪光覆盖 ──
    target.setView(originalView);
    if (m_anomalySystem.isActive(AnomalyType::Flash)) {
        float fi = m_anomalySystem.getIntensity(AnomalyType::Flash);
        m_flashOverlay.setFillColor(sf::Color(255, 255, 255, (std::uint8_t)(fi * 200)));
        target.draw(m_flashOverlay);
    }
}







