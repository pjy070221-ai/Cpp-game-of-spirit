#include "GameplayScene.h"
#include <cmath>

float GameplayScene::getTrackCenterX(int track) const {
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth
                   + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;
    return startX + track * (m_trackWidth + m_trackSpacing) + m_trackWidth / 2.0f;
}

void GameplayScene::render(sf::RenderTarget& target) {
    // 异象：ScreenShake（全屏抖动）+ LaneShift（轨道横向抖动），可叠加
    sf::View originalView = target.getView();
    float totalOx = 0.0f, totalOy = 0.0f;
    {
        static std::mt19937 rng(std::random_device{}());
        if (m_anomalySystem.isActive(AnomalyType::ScreenShake)) {
            float intensity = m_anomalySystem.getIntensity(AnomalyType::ScreenShake);
            float ms = 5.0f * intensity;  // 较轻的屏幕抖动
            totalOx += std::uniform_real_distribution<float>(-ms, ms)(rng);
            totalOy += std::uniform_real_distribution<float>(-ms, ms)(rng);
        }
        if (m_laneShakeBurst > 0.001f) {
            // 定向脉冲：先 slam 到一侧，再 ease-out 衰减回中
            float dir = m_laneShakeDirection;  // +1 右 / -1 左
            float offset = dir * 60.0f * m_laneShakeBurst * m_laneShakeBurst;  // ease-out 二次衰减
            totalOx += offset;
        }
    }
    if (totalOx != 0.0f || totalOy != 0.0f) {
        sf::View shifted = originalView;
        shifted.move({totalOx, totalOy});
        target.setView(shifted);
    }

    // 背景图（如有，替换渐变背景）
    if (m_hasBackground && m_bgSprite.has_value()) {
        target.draw(*m_bgSprite);
        // 暗色遮罩压暗背景，保持氛围
        sf::RectangleShape bgOverlay({m_screenWidth, m_screenHeight});
        bgOverlay.setFillColor(sf::Color(0, 0, 0, 100));
        target.draw(bgOverlay);
    }

    // 背景脉冲呼吸效果（无背景图时使用）
    if (!m_hasBackground) {
        float glow = m_glowIntensity * 30.0f;
        sf::Color topColor((int)(10 + glow * 0.3f), (int)(5 + glow * 0.2f), (int)(20 + glow * 0.5f));
        sf::Color botColor((int)(20 + glow * 0.5f), (int)(10 + glow * 0.3f), (int)(40 + glow));
        m_bgGradient[0].color = topColor;
        m_bgGradient[1].color = topColor;
        m_bgGradient[2].color = botColor;
        m_bgGradient[3].color = botColor;
        target.draw(m_bgGradient);
    }

    // 轨道霓虹配色
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

        // 根据接近判定线的音符计算轨道发光强度
        float trackGlow = 0.0f;
        for (auto& nr : m_noteRuntimes) {
            if (!nr.active || nr.processed || nr.track != t) continue;
            float dist = std::abs(nr.y - m_judgmentLineY);
            if (dist < 200.0f) trackGlow = std::max(trackGlow, 1.0f - dist / 200.0f);
        }

        // 1. 轨道外层辉光
        float outerGlowA = 15.0f + trackGlow * 45.0f;
        sf::RectangleShape outerGlow({m_trackWidth + 20.0f, m_judgmentLineY - 50.0f});
        outerGlow.setPosition({trackX - 10.0f, 50.0f});
        outerGlow.setFillColor(sf::Color(nc.r, nc.g, nc.b, (int)outerGlowA));
        target.draw(outerGlow);

        // 2. 轨道主体（增强）
        float g = 60.0f + trackGlow * 120.0f;
        m_tracks[t].setFillColor(sf::Color((int)(30 + g * 0.3f), (int)(20 + g * 0.5f),
                                            (int)(50 + g * 0.8f), (int)(80 + trackGlow * 80)));
        target.draw(m_tracks[t]);

        // 3. 轨道霓虹边框（左 + 右）
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

        // 4. 轨道底部发光区域（增强）
        float glowA = 20.0f + trackGlow * 80.0f;
        sf::RectangleShape gz({m_trackWidth, 80.0f});
        gz.setPosition({trackX, m_judgmentLineY - 80.0f});
        gz.setFillColor(sf::Color(nc.r, nc.g, nc.b, (int)glowA));
       target.draw(gz);
   }

    // Hold 长条 — 已判定（processed）时隐藏。渲染与判定生命周期同步。
    for (size_t i = 0; i < m_holdBars.size() && i < m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        if (m_noteRuntimes[i].type != 1) continue;
        target.draw(m_holdBars[i]);
    }

   // 音符拖尾（每个音符上方的残影矩形）
   for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        // 跳过被按住的 Hold 音符（由 Hold 长条显示，非音符头部 + 拖尾）
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

    // Tap 音符（跳过被按住的 Hold — 由 Hold 长条表示）
   for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (m_noteRuntimes[i].active && !m_noteRuntimes[i].processed) {
            // 跳过 Hold 头部（Hold 长条已显示）
            if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
            target.draw(m_activeShapes[i]);
        }
   }

    // HP 血量条
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

    // 命中粒子特效
    m_hitFX.render(target);

    // 判定线（带脉冲效果）
    uint8_t pulseAlpha = (uint8_t)(128 + (int)(127 * m_glowIntensity));
    m_judgmentLineShape.setFillColor(sf::Color(0, 255, 255, pulseAlpha));
    m_judgmentLineShape.setSize({m_screenWidth, 3.0f + m_glowIntensity});
    target.draw(m_judgmentLineShape);

    // 得分弹出文字
    for (auto& sp : m_scorePopups) if (sp.text.has_value()) target.draw(*sp.text);

    // 判定光环
    for (auto& hr : m_hitRings) target.draw(hr.shape);

    // Combo 闪光
    if (m_comboFlashTimer > 0) {
        float fa = m_comboFlashTimer / 0.15f * 180;
        m_comboFlashOverlay.setFillColor(sf::Color(255, 255, 255, (uint8_t)fa));
        target.draw(m_comboFlashOverlay);
    }

    // HUD 信息
    if (m_scoreText.has_value()) {
        m_scoreText->setPosition({20.0f, 10.0f});
        target.draw(*m_scoreText);
    }
    if (m_comboText.has_value() && m_combo > 0) {
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
    // 歌名 & 作者 — 右上角 AUTO PLAY 正下方
    if (m_songTitleText.has_value()) {
        m_songTitleText->setPosition({m_screenWidth - 20.0f, 45.0f});
        m_songTitleText->setOrigin({m_songTitleText->getLocalBounds().size.x, 0.0f});  // 右对齐
        target.draw(*m_songTitleText);
    }

    // 进度条
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

    // 倒计时遮罩
    if (m_countdownState == CountdownState::Counting) {
        sf::RectangleShape cdOverlay({m_screenWidth, m_screenHeight});
        cdOverlay.setFillColor(sf::Color(0, 0, 0, 140));
        target.draw(cdOverlay);
        if (m_cdNumText.has_value()) target.draw(*m_cdNumText);
        if (m_cdGoText.has_value()) target.draw(*m_cdGoText);
    }

    // 自动演奏提示
    if (m_autoPlay) {
        sf::Text autoText(*m_font, "AUTO PLAY", 28);
        autoText.setFillColor(sf::Color(255, 200, 50, 200));
        autoText.setPosition({m_screenWidth - 180.0f, 10.0f});
        target.draw(autoText);
    }

    // 钟声暗色脉冲
    if (m_bellTollAlpha > 1.0f) {
        sf::RectangleShape bellOverlay({m_screenWidth, m_screenHeight});
        bellOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)m_bellTollAlpha));
        target.draw(bellOverlay);
    }

    // 演出字幕：暗色遮罩 + 逐词文字
    if (m_cineActive && m_cineDarkAlpha > 1.0f) {
        sf::RectangleShape cineOverlay({m_screenWidth, m_screenHeight});
        cineOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)m_cineDarkAlpha));
        target.draw(cineOverlay);
        if (m_cineText.has_value()) target.draw(*m_cineText);
    }

    // 恢复视图 + 闪光异象遮罩
    target.setView(originalView);
    if (m_anomalySystem.isActive(AnomalyType::Flash)) {
        float fi = m_anomalySystem.getIntensity(AnomalyType::Flash);
        m_flashOverlay.setFillColor(sf::Color(255, 255, 255, (uint8_t)(fi * 200)));
        target.draw(m_flashOverlay);
    }
}

