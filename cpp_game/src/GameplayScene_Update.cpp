#include "GameplayScene.h"
#include "PauseScene.h"
#include <cmath>

void GameplayScene::update(float dt) {
    // 閳光偓閳光偓 閸婃帟顓搁弮?3-2-1 閳光偓閳光偓
    if (m_countdownState == CountdownState::Counting) {
        m_countdownTimer -= dt;
        if (m_countdownTimer <= 0.0f) {
            m_countdownState = CountdownState::Started;
            m_cdNumText.reset(); m_cdGoText.reset(); m_cdLastDisplayed = -1;
            if (m_countdownShouldStart) { m_countdownShouldStart = false; m_isPlaying = true; m_musicPlayer.play(); }
            else { m_isPlaying = true; m_musicPlayer.play(); }
        } else {
            int disp = 0;
            if (m_countdownTimer > 2.5f) disp = 3;
            else if (m_countdownTimer > 1.5f) disp = 2;
            else if (m_countdownTimer > 0.5f) disp = 1;
            if (disp != m_cdLastDisplayed) {
                m_cdLastDisplayed = disp; m_cdNumText.reset(); m_cdGoText.reset();
                if (disp > 0) {
                    m_cdNumText.emplace(m_font, std::to_string(disp), 130);
                    auto b = m_cdNumText->getLocalBounds();
                    m_cdNumText->setOrigin({b.size.x/2, b.size.y/2});
                    m_cdNumText->setPosition({m_screenWidth/2, m_screenHeight/2});
                    m_cdNumText->setFillColor(sf::Color::White);
                } else {
                    m_cdGoText.emplace(m_font, L"\x5F00\x59CB!", 80);
                    auto b = m_cdGoText->getLocalBounds();
                    m_cdGoText->setOrigin({b.size.x/2, b.size.y/2});
                    m_cdGoText->setPosition({m_screenWidth/2, m_screenHeight/2});
                    m_cdGoText->setFillColor(sf::Color(255,255,100));
                }
            }
        }
        return;
    }

    // Return to Menu 娴兼ê鍘涙径鍕倞
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

    if (!m_isPlaying || m_songFinished) return;

    m_pulseTime += dt;
    m_glowIntensity = 0.5f + 0.3f * std::sin(m_pulseTime * 2.0f);

    float currentTime = 0.0f;
    if (m_musicPlayer.isLoaded()) {
        currentTime = m_musicPlayer.getCurrentTime();
    } else if (m_isPlaying) {
        // 閺冪娀鐓舵稊鎰閻劍膩閹风喐妞傞柦鐔稿腹鏉?
        m_simTime += dt;
        currentTime = m_simTime;
    }

    // 瀵倽钖勭化鑽ょ埠閺囧瓨鏌?
    m_anomalySystem.update(currentTime, dt);

    // 鐠侊紕鐣婚張澶嬫櫏娑撳鎯ら柅鐔峰閿涘牆褰?NoteSpeedChange 瀵倽钖勮ぐ鍗炴惙閿?
    float effSpeed = m_noteSpeedPixels;
    if (m_anomalySystem.isActive(AnomalyType::NoteSpeedChange)) {
        float speedMult = m_anomalySystem.getParam("speed", 0.5f);
        effSpeed *= speedMult;
    }

        spawnNotes(currentTime, effSpeed);

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
        auto& nr = m_noteRuntimes[i];
       if (nr.isHeld) {
            // pressed: head locks at judgment line, bar shrinks from above
           float remaining = nr.targetTime + nr.holdDuration - currentTime;
            float tailY = m_judgmentLineY - remaining * effSpeed; // above judgment line
            float barH = m_judgmentLineY - tailY;
           if (barH < 0.0f) barH = 0.0f;
            m_holdBars[i].setPosition({cx - m_trackWidth / 2.0f + 4.0f, tailY});
           m_holdBars[i].setSize({m_trackWidth - 8.0f, barH});
           m_holdBars[i].setFillColor(sf::Color(0, 255, 200, 140));
       } else {
           // unpressed: full-length bar from tail to head
           float tailY = m_judgmentLineY - (nr.targetTime + nr.holdDuration - currentTime) * effSpeed;
            float barH = nr.holdDuration * effSpeed;
           m_holdBars[i].setPosition({cx - m_trackWidth / 2.0f + 4.0f, tailY});
           m_holdBars[i].setSize({m_trackWidth - 8.0f, barH});
           m_holdBars[i].setFillColor(sf::Color(0, 200, 255, 60));
       }
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

    // 閳光偓閳光偓 閸掑棙鏆熷鐟板毉 閳光偓閳光偓
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

    // 閳光偓閳光偓 閸掋倕鐣鹃崗澶屽箚 閳光偓閳光偓
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

    // 閳光偓閳光偓 鏉╃偛鍤梻顏勫帨 閳光偓閳光偓
    if (m_comboFlashTimer > 0) m_comboFlashTimer -= dt;

    // 閼哄倸顨旀径褍绗€ HP 瑜版帡娴?閳?濞撳憡鍨欑紒鎾存将
    if (m_hp <= 0 && m_isPlaying) {
        m_isPlaying = false;
        m_songFinished = true;
        endGame();
        return;
    }

    if (m_noteIndex >= (int)m_noteData.size() && allNotesProcessed())
        endGame();
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
            nr.noteSpeed = effSpeed;
           float timeUntilHit = src.time - currentTime;
            nr.y = m_judgmentLineY - effSpeed * timeUntilHit;
           nr.active = true;
            nr.holdDuration = src.holdDuration;
            nr.isHeld = false;
            m_noteRuntimes.push_back(nr);

            // 閼哄倸顨旀径褍绗€: 閻晛鑸伴棅宕囶儊 + 鏉炪劑浜鹃悪顒傜彌妫版粏澹?
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








