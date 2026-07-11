#include "GameplayScene.h"
#include <cmath>

void GameplayScene::checkJudgment(int track) {
    float minDist = 9999.0f;
    int bestIdx = -1;
    for (int i = 0; i < (int)m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        if (m_noteRuntimes[i].track != track) continue;
        // skip hold notes that are already being held
        if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
        float dist = std::abs(m_noteRuntimes[i].y - m_judgmentLineY);
        if (dist < minDist) { minDist = dist; bestIdx = i; }
    }
    if (bestIdx < 0) return;

    auto& nr = m_noteRuntimes[bestIdx];

    // hold note press
    if (nr.type == 1) {
        float dist = std::abs(nr.y - m_judgmentLineY);
        if (dist > m_goodWindow) return; // too early or too late, ignore
        nr.isHeld = true;
        m_holdBars[bestIdx].setFillColor(sf::Color(0, 255, 200, 120));
        // score for the successful press
        nr.processed = true; // marks the head hit
        if (dist < m_perfectWindow) {
            m_score += 50; m_perfectCount++;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("HOLD");
                m_judgmentText->setFillColor(sf::Color::Yellow);
            }
        } else if (dist < m_greatWindow) {
            m_score += 30; m_greatCount++;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("HOLD");
                m_judgmentText->setFillColor(sf::Color::Cyan);
            }
        } else {
            m_score += 15; m_goodCount++;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("HOLD");
                m_judgmentText->setFillColor(sf::Color::Green);
            }
        }
        m_combo++;
        if (m_combo > m_maxCombo) m_maxCombo = m_combo;
        m_judgmentDisplayTimer = 0.8f;
        if (m_scoreText.has_value())
            m_scoreText->setString("Score: " + std::to_string(m_score));
        if (m_comboText.has_value())
            m_comboText->setString(std::to_string(m_combo));
        // un-processed so update() can track hold completion
        nr.processed = false;
        return;
    }

    m_lastHitTrack = track;
    // normal note judgment
    JudgeResult result;
    if      (minDist < m_perfectWindow) result = JudgeResult::Perfect;
    else if (minDist < m_greatWindow)   result = JudgeResult::Great;
    else if (minDist < m_goodWindow)    result = JudgeResult::Good;
    else                                result = JudgeResult::Miss;

    onNoteJudged(result);
    nr.processed = true;
}

void GameplayScene::checkHoldRelease(int track) {
    for (auto& nr : m_noteRuntimes) {
        if (!nr.active || nr.track != track) continue;
        if (nr.type == 1 && nr.isHeld && !nr.processed) {
            // released early = miss
            nr.isHeld = false;
            nr.processed = true;
            m_missCount++;
            m_combo = 0;
            m_hp -= 5;
            m_hp -= 8;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("BREAK");
                m_judgmentText->setFillColor(sf::Color::Red);
            }
            m_judgmentDisplayTimer = 0.8f;
        }
    }
}

void GameplayScene::onNoteJudged(JudgeResult result) {
    switch (result) {
    case JudgeResult::Perfect: {
        m_score += 100; m_combo++; m_perfectCount++;
        if (m_judgmentText.has_value()) m_judgmentText->setString("PERFECT");
        m_lastJudgmentColor = sf::Color::Yellow;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 25, sf::Color::Yellow, 60, 250, 0.3f, 0.8f, 2.0f, 7.0f);
        break;
    }
        break;
    case JudgeResult::Great: {
        m_score += 50; m_combo++; m_greatCount++;
        if (m_judgmentText.has_value()) m_judgmentText->setString("GREAT");
        m_lastJudgmentColor = sf::Color::Cyan;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 20, sf::Color::Cyan, 50, 200, 0.3f, 0.7f, 2.0f, 6.0f);
        break;
    }
        break;
    case JudgeResult::Good: {
        m_score += 25; m_combo++; m_goodCount++; m_hp -= 3;
        if (m_judgmentText.has_value()) m_judgmentText->setString("GOOD");
        m_lastJudgmentColor = sf::Color::Green;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 15, sf::Color::Green, 40, 160, 0.3f, 0.6f, 2.0f, 5.0f);
        break;
    }
        break;
    case JudgeResult::Miss: {
        m_missCount++; m_combo = 0; m_hp -= 8;
        if (m_judgmentText.has_value()) m_judgmentText->setString("MISS");
        m_lastJudgmentColor = sf::Color::Red;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 8, sf::Color::Red, 30, 100, 0.2f, 0.5f, 1.0f, 4.0f);
        break;
    }
        break;
    default: return;
    }
    m_judgmentDisplayTimer = 0.8f;
    m_judgmentText->setFillColor(m_lastJudgmentColor);
    if (m_combo > m_maxCombo) m_maxCombo = m_combo;

    if (m_scoreText.has_value())
        m_scoreText->setString("Score: " + std::to_string(m_score));
    if (m_comboText.has_value())
        m_comboText->setString(std::to_string(m_combo));
}

void GameplayScene::autoMissCheck() {
    float missThreshold = m_judgmentLineY + m_goodWindow + 50.0f;
    for (auto& nr : m_noteRuntimes) {
        if (!nr.active || nr.processed) continue;
        // don't auto-miss notes being held
        if (nr.type == 1 && nr.isHeld) continue;
        if (nr.y > missThreshold) {
            nr.processed = true;
            m_missCount++;
            m_combo = 0;
            m_hp -= 8;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("MISS");
                m_judgmentText->setFillColor(sf::Color::Red);
            }
            m_judgmentDisplayTimer = 0.8f;
        }
    }
}

bool GameplayScene::allNotesProcessed() const {
    for (const auto& nr : m_noteRuntimes)
        if (nr.active && !nr.processed) return false;
    return true;
}





