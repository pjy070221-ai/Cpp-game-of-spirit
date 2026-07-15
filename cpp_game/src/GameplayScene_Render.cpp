#include "GameplayScene.h"
#include <cmath>

float GameplayScene::getTrackCenterX(int track) const {
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth
                   + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;
    return startX + track * (m_trackWidth + m_trackSpacing) + m_trackWidth / 2.0f;
}

void GameplayScene::render(sf::RenderTarget& target) {
    // screen shake
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

    // background pulse
    float glow = m_glowIntensity * 30.0f;
    sf::Color topColor((int)(10 + glow * 0.3f), (int)(5 + glow * 0.2f), (int)(20 + glow * 0.5f));
    sf::Color botColor((int)(20 + glow * 0.5f), (int)(10 + glow * 0.3f), (int)(40 + glow));
    m_bgGradient[0].color = topColor;
    m_bgGradient[1].color = topColor;
    m_bgGradient[2].color = botColor;
    m_bgGradient[3].color = botColor;
    target.draw(m_bgGradient);

    // track neon colors
    static const sf::Color neonColors[4] = {
        sf::Color(0, 220, 255),
        sf::Color(255, 100, 200),
        sf::Color(255, 210, 0),
        sf::Color(100, 230, 100)
    };
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth
                   + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;

    for (int t = 0; t < m_trackCount; ++t) {
        float trackX = startX + t * (m_trackWidth + m_trackSpacing);
        const auto& nc = neonColors[t];

        // compute glow intensity from approaching notes
        float trackGlow = 0.0f;
        for (auto& nr : m_noteRuntimes) {
            if (!nr.active || nr.processed || nr.track != t) continue;
            float dist = std::abs(nr.y - m_judgmentLineY);
            if (dist < 200.0f) trackGlow = std::max(trackGlow, 1.0f - dist / 200.0f);
        }

        // 1. outer glow behind track
        float outerGlowA = 15.0f + trackGlow * 45.0f;
        sf::RectangleShape outerGlow({m_trackWidth + 20.0f, m_judgmentLineY - 50.0f});
        outerGlow.setPosition({trackX - 10.0f, 50.0f});
        outerGlow.setFillColor(sf::Color(nc.r, nc.g, nc.b, (int)outerGlowA));
        target.draw(outerGlow);

        // 2. track body (enhanced)
        float g = 60.0f + trackGlow * 120.0f;
        m_tracks[t].setFillColor(sf::Color((int)(30 + g * 0.3f), (int)(20 + g * 0.5f),
                                            (int)(50 + g * 0.8f), (int)(80 + trackGlow * 80)));
        target.draw(m_tracks[t]);

        // 3. neon track border lines (left + right)
        float lineGlowBase = 120.0f + trackGlow * 135.0f;
        int rL = (int)std::min(255.0f, nc.r * lineGlowBase / 180.0f);
        int gL = (int)std::min(255.0f, nc.g * lineGlowBase / 180.0f);
        int bL = (int)std::min(255.0f, nc.b * lineGlowBase / 180.0f);
        int aL = (int)(180 + trackGlow * 75);
        sf::Color lineColor(rL, gL, bL, aL);

        sf::RectangleShape borderL({3.0f, m_judgmentLineY - 50.0f});
        borderL.setPosition({trackX, 50.0f});
        borderL.setFillColor(lineColor);
        target.draw(borderL);

        sf::RectangleShape borderR({3.0f, m_judgmentLineY - 50.0f});
        borderR.setPosition({trackX + m_trackWidth - 3.0f, 50.0f});
        borderR.setFillColor(lineColor);
        target.draw(borderR);

        // 4. track bottom glow zone (enhanced)
        float glowA = 20.0f + trackGlow * 80.0f;
        sf::RectangleShape gz({m_trackWidth, 80.0f});
        gz.setPosition({trackX, m_judgmentLineY - 80.0f});
        gz.setFillColor(sf::Color(nc.r, nc.g, nc.b, (int)glowA));
       target.draw(gz);
   }

    // milestone glow overlay [DISABLED]
    // if (m_milestoneGlowTimer > 0) {
    //     float msGlow = m_milestoneGlowTimer / 0.5f;
    //     m_milestoneGlowOverlay.setFillColor(sf::Color(255, 255, 255, (uint8_t)(msGlow * 100)));
    //     target.draw(m_milestoneGlowOverlay);
    // }

    // hold bars - hide when judged (sync'ed with judgment lifecycle)
    for (size_t i = 0; i < m_holdBars.size() && i < m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        if (m_noteRuntimes[i].type != 1) continue;
        target.draw(m_holdBars[i]);
    }

   // note trail (ghost rectangles above each note)
   for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        // skip held hold notes (shown by hold bar, not by note head + trail)
        if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
       const auto& shape = m_activeShapes[i];
        const auto& nr = m_noteRuntimes[i];
        auto col = shape.getFillColor();
        sf::Vector2f pos = shape.getPosition();
        for (int t = 1; t <= 5; ++t) {
            int alpha = 55 - t * 10;
            if (alpha <= 0) continue;
            sf::RectangleShape trail = shape;
            trail.setPosition({pos.x, pos.y - t * 7.0f});
            trail.setFillColor(sf::Color(col.r, col.g, col.b, (uint8_t)alpha));
            target.draw(trail);
        }
    }

    // tap notes (skip held hold notes - shown by hold bar)
   for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (m_noteRuntimes[i].active && !m_noteRuntimes[i].processed) {
            // skip head of held hold notes (the hold bar shows it)
            if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
            target.draw(m_activeShapes[i]);
        }
   }

    m_hitFX.render(target);
    m_comboFX.render(target);

    // judgment line with pulse
    uint8_t pulseAlpha = (uint8_t)(128 + (int)(127 * m_glowIntensity));
    m_judgmentLineShape.setFillColor(sf::Color(0, 255, 255, pulseAlpha));
    m_judgmentLineShape.setSize({m_screenWidth, 3.0f + m_glowIntensity});
    target.draw(m_judgmentLineShape);

    m_milestoneFX.render(target);

    // score popups
    for (auto& sp : m_scorePopups) if (sp.text.has_value()) target.draw(*sp.text);

    // hit rings
    for (auto& hr : m_hitRings) target.draw(hr.shape);

    // combo flash
    if (m_comboFlashTimer > 0) {
        float fa = m_comboFlashTimer / 0.15f * 180;
        m_comboFlashOverlay.setFillColor(sf::Color(255, 255, 255, (uint8_t)fa));
        target.draw(m_comboFlashOverlay);
    }

    // HUD
    if (m_scoreText.has_value()) {
        m_scoreText->setPosition({20.0f, 10.0f});
        target.draw(*m_scoreText);
    }

    // combo milestone ring
    if (m_comboRingTimer > 0) {
        float t = 1.0f - m_comboRingTimer / 0.6f;
        float radius = 20.0f + t * 50.0f;
        sf::CircleShape ring(radius);
        ring.setPosition({640.0f - radius, 600.0f - radius});
        ring.setFillColor(sf::Color::Transparent);
        sf::Color rc = m_comboPopColor;
        rc.a = (uint8_t)((1.0f - t) * 200);
        ring.setOutlineThickness(2.0f + t * 2.0f);
        ring.setOutlineColor(rc);
        target.draw(ring);
    }

    if (m_comboText.has_value() && m_combo > 0) {
        float cs = 1.0f;
        if      (m_combo >= 50) cs = 2.0f;
        else if (m_combo >= 30) cs = 1.6f;
        else if (m_combo >= 10) cs = 1.2f;
        cs *= m_comboPopScale;
        m_comboText->setScale({cs, cs});
        if (m_comboPopTimer > 0)
            m_comboText->setFillColor(m_comboPopColor);
        else
            m_comboText->setFillColor(sf::Color::White);
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

    // progress bar
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

    // countdown overlay
    if (m_countdownState == CountdownState::Counting) {
        sf::RectangleShape cdOverlay({m_screenWidth, m_screenHeight});
        cdOverlay.setFillColor(sf::Color(0, 0, 0, 140));
        target.draw(cdOverlay);
        if (m_cdNumText.has_value()) target.draw(*m_cdNumText);
        if (m_cdGoText.has_value()) target.draw(*m_cdGoText);
    }

    // restore view + flash overlay
    target.setView(originalView);
    if (m_anomalySystem.isActive(AnomalyType::Flash)) {
        float fi = m_anomalySystem.getIntensity(AnomalyType::Flash);
        m_flashOverlay.setFillColor(sf::Color(255, 255, 255, (uint8_t)(fi * 200)));
        target.draw(m_flashOverlay);
    }
}



