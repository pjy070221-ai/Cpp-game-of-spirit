#include "GameplayScene.h"
#include "PauseScene.h"
#include <cmath>

void GameplayScene::update(float dt) {
    // ======== 倒计时 3-2-1 开场 ========
    if (m_countdownState == CountdownState::Counting) {
        m_countdownTimer -= dt;
        if (m_countdownTimer <= 0.0f) {
            m_countdownState = CountdownState::Started;
            m_cdNumText.reset(); m_cdGoText.reset(); m_cdLastDisplayed = -1;
            if (m_countdownShouldStart) m_countdownShouldStart = false;
            m_isPlaying = true; m_musicPlayer.play();
        } else {
            int disp = 0;
            if (m_countdownTimer > 2.5f) disp = 3;
            else if (m_countdownTimer > 1.5f) disp = 2;
            else if (m_countdownTimer > 0.5f) disp = 1;
            if (disp != m_cdLastDisplayed) {
                m_cdLastDisplayed = disp; m_cdNumText.reset(); m_cdGoText.reset();
                if (disp > 0) {
                    m_cdNumText.emplace(*m_font, std::to_string(disp), 130);
                    auto b = m_cdNumText->getLocalBounds();
                    m_cdNumText->setOrigin({b.size.x/2, b.size.y/2});
                    m_cdNumText->setPosition({m_screenWidth/2, m_screenHeight/2});
                    m_cdNumText->setFillColor(sf::Color::White);
                } else {
                    m_cdGoText.emplace(*m_font, L"\x5F00\x59CB!", 80);
                    auto b = m_cdGoText->getLocalBounds();
                    m_cdGoText->setOrigin({b.size.x/2, b.size.y/2});
                    m_cdGoText->setPosition({m_screenWidth/2, m_screenHeight/2});
                    m_cdGoText->setFillColor(sf::Color(255,255,100));
                }
            }
        }
        return;
    }

    // 返回主菜单（来自 PauseScene 信号）
    if (s_returnToMenu) {
        s_returnToMenu = false;
        m_musicPlayer.stop();
        m_isPlaying = false;
        m_songFinished = true;
        requestPop();
        return;
    }

        if (s_retry) {
        s_retry = false;
        requestReplace(std::make_unique<GameplayScene>());
        return;
    }

    // ======== 结算延迟：等待 2 秒后再切换到结算界面 ========
    if (m_endDelayTimer > 0.0f) {
        m_endDelayTimer -= dt;
        if (m_endDelayTimer <= 0.0f) {
            endGame();
            return;
        }
    }

    if (!m_isPlaying || m_songFinished) return;

    m_pulseTime += dt;
    m_glowIntensity = 0.5f + 0.3f * std::sin(m_pulseTime * 2.0f);

    float currentTime = 0.0f;
    if (m_musicPlayer.isLoaded()) {
        currentTime = m_musicPlayer.getCurrentTime();
    } else if (m_isPlaying) {
        // 音乐未加载时使用模拟时间驱动
        m_simTime += dt;
        currentTime = m_simTime;
    }

    // 异象系统更新
    m_anomalySystem.update(currentTime, dt);

    // LaneShift 定向脉冲：事件激活边沿触发，方向交替，强度自衰减
    bool laneShiftActive = m_anomalySystem.isActive(AnomalyType::LaneShift);
    if (laneShiftActive && !m_wasLaneShiftActive) {
        m_laneShakeBurst = 1.0f;           // 新爆发
        m_laneShakeDirection *= -1.0f;     // 方向交替
    }
    m_wasLaneShiftActive = laneShiftActive;
    if (m_laneShakeBurst > 0.0f) {
        m_laneShakeBurst = std::max(0.0f, m_laneShakeBurst - dt * 4.0f);  // ~0.25s 衰减
    }

    // ======== 钟声脉冲（57-58.5s 三声钟声 + 玻璃碎裂）========
    {
        static const float bellTimes[] = {57.0f, 57.65f, 58.3f};
        for (float bt : bellTimes) {
            float dist = currentTime - bt;
            if (dist >= 0.0f && dist < 0.3f) {
                // 钟声：快速暗色脉冲，ease-out 衰减
                float p = 1.0f - dist / 0.3f;
                float alpha = 140.0f * p * p;  // quadratic ease-out
                if (alpha > m_bellTollAlpha) m_bellTollAlpha = alpha;
            }
        }
        if (m_bellTollAlpha > 0.0f)
            m_bellTollAlpha = std::max(0.0f, m_bellTollAlpha - dt * 500.0f);  // 快速衰减
    }

    // ======== 演出字幕（117s "Tell me, what scares you."）========
    if (!m_cineActive && currentTime >= 117.0f && currentTime < 120.0f && m_font) {
        m_cineActive = true;
        m_cineTimer = 0.0f;
        m_cineWordIdx = 0;
        m_cineDarkAlpha = 0.0f;
        m_cineWords = {L"Tell", L"me,", L"what", L"scares", L"you."};
        m_cineText.emplace(*m_font, L"", 52);
        m_cineText->setFillColor(sf::Color(255, 255, 255, 0));
        auto b = m_cineText->getLocalBounds();
        m_cineText->setPosition({640.0f, 80.0f});
    }
    if (m_cineActive) {
        m_cineTimer += dt;
        float t = m_cineTimer;
        // 阶段：0→0.8s 渐暗 / 0.8→2.5s 显示文字 / 2.5→3.3s 渐明
        if (t < 0.8f) {
            // ease-in 渐暗
            float p = t / 0.8f;
            m_cineDarkAlpha = 180.0f * (p * p);  // quadratic ease-in
        } else if (t < 2.5f) {
            m_cineDarkAlpha = 180.0f;
        } else if (t < 3.3f) {
            // ease-out 渐明
            float p = (t - 2.5f) / 0.8f;
            m_cineDarkAlpha = 180.0f * (1.0f - p * p);
        } else {
            m_cineActive = false;
            m_cineDarkAlpha = 0.0f;
        }

        // 逐词显示：0.8s 后开始，每 0.35s 一个词
        float wordTime = t - 0.8f;
        if (wordTime > 0.0f) {
            int newIdx = std::min((int)m_cineWords.size(), (int)(wordTime / 0.35f));
            if (newIdx != m_cineWordIdx && newIdx <= (int)m_cineWords.size()) {
                m_cineWordIdx = newIdx;
                sf::String full;
                for (int i = 0; i < m_cineWordIdx; ++i) {
                    if (i > 0) full += L" ";
                    full += m_cineWords[i];
                }
                if (m_cineText.has_value()) {
                    m_cineText->setString(full);
                    auto b = m_cineText->getLocalBounds();
                    m_cineText->setOrigin({b.size.x / 2.0f, 0.0f});
                    m_cineText->setFillColor(sf::Color(255, 255, 255, 220));
                }
            }
        }
    }

    // 流速受 NoteSpeedChange 异象影响时动态调整
    float effSpeed = m_noteSpeedPixels;
    if (m_anomalySystem.isActive(AnomalyType::NoteSpeedChange)) {
        float speedMult = m_anomalySystem.getParam("speed", 0.5f);
        effSpeed *= speedMult;
    }

        spawnNotes(currentTime, effSpeed);

    for (auto& nr : m_noteRuntimes) {
        if (nr.active && !nr.processed)
            nr.y = m_judgmentLineY - (nr.targetTime - currentTime) * effSpeed;
    }

    for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (m_noteRuntimes[i].active)
            m_activeShapes[i].setPosition(
                {getTrackCenterX(m_noteRuntimes[i].track), m_noteRuntimes[i].y});
    }

   // 更新 Hold 长条位置
   for (size_t i = 0; i < m_holdBars.size() && i < m_noteRuntimes.size(); ++i) {
       if (!m_noteRuntimes[i].active) continue;
       if (m_noteRuntimes[i].type != 1) continue;
       float cx = getTrackCenterX(m_noteRuntimes[i].track);
        auto& nr = m_noteRuntimes[i];
       if (nr.isHeld) {
            // 已按住：头部锁定在判定线，长条从上方收缩
           float remaining = nr.targetTime + nr.holdDuration - currentTime;
            float tailY = m_judgmentLineY - remaining * effSpeed; // 判定线上方
            float barH = m_judgmentLineY - tailY;
           if (barH < 0.0f) barH = 0.0f;
            m_holdBars[i].setPosition({cx - m_trackWidth / 2.0f + 4.0f, tailY});
           m_holdBars[i].setSize({m_trackWidth - 8.0f, barH});
           m_holdBars[i].setFillColor(sf::Color(0, 255, 200, 240));
       } else {
           // 未按住：完整长条（从尾部到头部）
           float tailY = m_judgmentLineY - (nr.targetTime + nr.holdDuration - currentTime) * effSpeed;
            float barH = nr.holdDuration * effSpeed;
           m_holdBars[i].setPosition({cx - m_trackWidth / 2.0f + 4.0f, tailY});
           m_holdBars[i].setSize({m_trackWidth - 8.0f, barH});
           m_holdBars[i].setFillColor(sf::Color(0, 200, 255, 170));
       }
   }

    // 自动演奏：音符到达判定线时自动触发
    if (m_autoPlay) {
        for (auto& nr : m_noteRuntimes) {
            if (!nr.active || nr.processed) continue;
            float timeDiff = currentTime - nr.targetTime;
            // Tap 音符：在 perfect 窗口内自动击中
            if (nr.type == 0 && timeDiff >= -m_perfectTimeWindow && timeDiff <= m_perfectTimeWindow) {
                checkJudgment(nr.track);
            }
            // Hold 音符：在 perfect 窗口内自动按下（尾部由 hold completion check 结算）
            if (nr.type == 1 && !nr.isHeld && timeDiff >= -m_perfectTimeWindow && timeDiff <= m_perfectTimeWindow) {
                checkJudgment(nr.track);
            }
        }
    }

    autoMissCheck();

    // Hold 音符完成检测
    if (m_musicPlayer.isLoaded()) {
        float now = m_musicPlayer.getCurrentTime();
        for (auto& nr : m_noteRuntimes) {
            if (nr.active && nr.isHeld && !nr.processed) {
                if (now >= nr.targetTime + nr.holdDuration) {
                    nr.processed = true;
                    nr.isHeld = false;
                    m_score += 75;
                    m_combo++;
                    if (m_combo > m_maxCombo) m_maxCombo = m_combo;
                    if (m_scoreText.has_value())
                        m_scoreText->setString("Score: " + std::to_string(m_score));
                    if (m_comboText.has_value())
                        m_comboText->setString(std::to_string(m_combo));
                }
            }
        }
    }


    if (m_judgmentDisplayTimer > 0)
        m_judgmentDisplayTimer -= dt;

    m_hitFX.update(dt);

    // ======== 得分弹出动画更新 ========
    for (auto& sp : m_scorePopups) {
        sp.life -= dt;
        if (sp.life > 0) {
            sp.text->move({0, -dt * 80});
            auto col = sp.text->getFillColor();
            col.a = (std::uint8_t)(sp.life / 0.8f * 255);
            sp.text->setFillColor(col);
        }
    }
    m_scorePopups.erase(std::remove_if(m_scorePopups.begin(), m_scorePopups.end(),
        [](auto& s) { return s.life <= 0; }), m_scorePopups.end());

    // ======== 判定光环扩展动画 ========
    for (auto& hr : m_hitRings) {
        hr.life -= dt;
        if (hr.life > 0) {
            float sc = 1.0f + (1.0f - hr.life / 0.5f) * 4.0f;
            hr.shape.setScale({sc, sc});
            auto col = hr.shape.getOutlineColor();
            col.a = (std::uint8_t)(hr.life / 0.5f * 200);
            hr.shape.setOutlineColor(col);
        }
    }
    m_hitRings.erase(std::remove_if(m_hitRings.begin(), m_hitRings.end(),
        [](auto& h) { return h.life <= 0; }), m_hitRings.end());

    // ======== Combo 闪光计时衰减 ========
    if (m_comboFlashTimer > 0) m_comboFlashTimer -= dt;

    // HP 归零检查 → 触发歌曲失败
    if (m_hp <= 0 && m_isPlaying) {
        m_isPlaying = false;
        m_songFinished = true;
        endGame();
        return;
    }

    // 所有音符处理完毕 → 启动结算延迟（避免结算界面突然弹出）
    if (m_noteIndex >= (int)m_noteData.size() && allNotesProcessed()) {
        if (m_endDelayTimer < 0.0f) {
            m_endDelayTimer = 2.0f;   // 延迟 2 秒
            m_isPlaying = false;      // 停止接收输入
        }
    }
}

void GameplayScene::spawnNotes(float currentTime, float effSpeed) {
   while (m_noteIndex < (int)m_noteData.size() &&
          m_noteData[m_noteIndex].time <= currentTime + SPAWN_LOOKAHEAD) {
       if (m_noteData[m_noteIndex].time >= currentTime) {
           NoteData& src = m_noteData[m_noteIndex];
           NoteRuntime nr;
           nr.track = src.track;
           nr.targetTime = src.time;
           nr.type = src.type;
           float timeUntilHit = src.time - currentTime;
            nr.y = m_judgmentLineY - effSpeed * timeUntilHit;
           nr.active = true;
            nr.holdDuration = src.holdDuration;
            nr.isHeld = false;
            m_noteRuntimes.push_back(nr);

            // 音符配色：主色 + 边框色（4 轨各不同）
            static const sf::Color nc[4] = {
                sf::Color(0, 220, 255), sf::Color(255, 100, 200),
                sf::Color(255, 210, 0), sf::Color(100, 230, 100)
            };
            static const sf::Color no[4] = {
                sf::Color(0, 180, 220), sf::Color(200, 60, 160),
                sf::Color(200, 170, 0), sf::Color(60, 190, 60)
            };
            float nw = m_trackWidth - 10.0f;
            sf::RectangleShape note({nw, 28.0f});
            note.setFillColor(nc[src.track % 4]);
            note.setOutlineThickness(2.0f);
            note.setOutlineColor(no[src.track % 4]);
            note.setOrigin({nw / 2.0f, 14.0f});
            m_activeShapes.push_back(note);

            sf::RectangleShape bar;
            bar.setFillColor(sf::Color(0, 200, 255, 200));
            m_holdBars.push_back(bar);
        }
        m_noteIndex++;
    }
}

void GameplayScene::handleEvent(const sf::Event& event) {
    if (!m_isPlaying) return;

    // ESC 暂停始终可用（不受 AutoPlay 限制）
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape) {
            if (m_isPlaying && !m_songFinished) {
                m_musicPlayer.pause();
                requestPush(std::make_unique<PauseScene>());
                return;
            }
        }
    }

    if (m_autoPlay) return;  // 自动演奏模式：屏蔽玩家输入

    int track = -1;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scancode::D) track = 0;
        else if (key->scancode == sf::Keyboard::Scancode::F) track = 1;
        else if (key->scancode == sf::Keyboard::Scancode::J) track = 2;
        else if (key->scancode == sf::Keyboard::Scancode::K) track = 3;
        if (track >= 0) { m_keysHeld[track] = true; checkJudgment(track); }
    }

    if (const auto* key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->scancode == sf::Keyboard::Scancode::D) track = 0;
        else if (key->scancode == sf::Keyboard::Scancode::F) track = 1;
        else if (key->scancode == sf::Keyboard::Scancode::J) track = 2;
        else if (key->scancode == sf::Keyboard::Scancode::K) track = 3;
        if (track >= 0) { m_keysHeld[track] = false; checkHoldRelease(track); }
    }
}








