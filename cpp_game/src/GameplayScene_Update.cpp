#include "GameplayScene.h"
#include "PauseScene.h"
#include <cmath>

void GameplayScene::update(float dt) {
    if (!m_isPlaying || m_songFinished) return;

    // 节奏大师 重试：重启当前谱面
    if (s_retry) {
        s_retry = false;
        requestReplace(std::make_unique<GameplayScene>());
        return;
    }

    if (s_returnToMenu) {
        s_returnToMenu = false;
        m_musicPlayer.stop();
        m_isPlaying = false;
        m_songFinished = true;
        requestPop();
        return;
    }

    m_pulseTime += dt;
    m_glowIntensity = 0.5f + 0.3f * std::sin(m_pulseTime * 2.0f);

    float currentTime = 0.0f;
    if (m_musicPlayer.isLoaded()) {
        currentTime = m_musicPlayer.getCurrentTime();
    } else if (m_isPlaying) {
        // 无音乐时用模拟时钟推进
        m_simTime += dt;
        currentTime = m_simTime;
    }

    // 异象系统更新
    m_anomalySystem.update(currentTime, dt);

    // 计算有效下落速度（受 NoteSpeedChange 异象影响）
    float effSpeed = m_noteSpeedPixels;
    if (m_anomalySystem.isActive(AnomalyType::NoteSpeedChange)) {
        float speedMult = m_anomalySystem.getParam("speed", 0.5f);
        effSpeed *= speedMult;
    }

        spawnNotes(currentTime);

    for (auto& nr : m_noteRuntimes) {
        if (nr.active && !nr.processed)
            nr.y += effSpeed * dt;
    }

    for (size_t i = 0; i < m_activeShapes.size() && i < m_noteRuntimes.size(); ++i) {
        if (m_noteRuntimes[i].active)
            m_activeShapes[i].setPosition(
                {getTrackCenterX(m_noteRuntimes[i].track), m_noteRuntimes[i].y});
    }

    // update hold bar positions
    for (size_t i = 0; i < m_holdBars.size() && i < m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active) continue;
        if (m_noteRuntimes[i].type != 1) continue;
        float cx = getTrackCenterX(m_noteRuntimes[i].track);
        float noteY = m_noteRuntimes[i].y;
        float barHeight = m_judgmentLineY - noteY;
        m_holdBars[i].setPosition({cx - m_trackWidth / 2.0f + 4.0f, noteY});
        m_holdBars[i].setSize({m_trackWidth - 8.0f, barHeight});
    }

    autoMissCheck();

    // hold note completion check
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
    // 节奏大师 HP 归零 → 游戏结束
    if (m_hp <= 0 && m_isPlaying) {
        m_isPlaying = false;
        m_songFinished = true;
        endGame();
        return;
    }

    if (m_noteIndex >= (int)m_noteData.size() && allNotesProcessed())
        endGame();
}

void GameplayScene::spawnNotes(float currentTime) {
    while (m_noteIndex < (int)m_noteData.size() &&
           m_noteData[m_noteIndex].time <= currentTime + SPAWN_LOOKAHEAD) {
        if (m_noteData[m_noteIndex].time >= currentTime) {
            NoteData& src = m_noteData[m_noteIndex];
            NoteRuntime nr;
            nr.track = src.track;
            nr.targetTime = src.time;
            nr.type = src.type;
            nr.y = 100.0f;
            nr.noteSpeed = m_noteSpeedPixels;
            nr.active = true;
            nr.holdDuration = src.holdDuration;
            nr.isHeld = false;
            m_noteRuntimes.push_back(nr);

            // 节奏大师: 矩形音符 + 轨道独立颜色
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
            bar.setFillColor(sf::Color(0, 200, 255, 80));
            m_holdBars.push_back(bar);
        }
        m_noteIndex++;
    }
}

void GameplayScene::handleEvent(const sf::Event& event) {
    if (!m_isPlaying) return;

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::Escape || key->code == sf::Keyboard::Key::Escape) {
            if (m_isPlaying && !m_songFinished) {
                m_musicPlayer.pause();
                requestPush(std::make_unique<PauseScene>());
                return;
            }
        }
    }

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








