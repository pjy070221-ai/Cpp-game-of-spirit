#include "GameplayScene.h"
#include "PauseScene.h"
#include <cmath>

void GameplayScene::update(float dt) {
    if (!m_isPlaying || m_songFinished) return;
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
    if (m_musicPlayer.isLoaded())
        currentTime = m_musicPlayer.getCurrentTime();

    spawnNotes(currentTime);

    for (auto& nr : m_noteRuntimes) {
        if (nr.active && !nr.processed)
            nr.y += m_noteSpeedPixels * dt;
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

            sf::CircleShape note(22.0f);
            note.setFillColor(sf::Color(0, 255, 255, 220));
            note.setOutlineThickness(2.0f);
            note.setOutlineColor(sf::Color(0, 200, 255));
            note.setOrigin({22.0f, 22.0f});
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




