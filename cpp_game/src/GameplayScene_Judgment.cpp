#include "GameplayScene.h"
#include <cmath>

void GameplayScene::checkJudgment(int track) {
    //
    float currentTime = m_simTime;
    if (m_musicPlayer.isLoaded())
        currentTime = m_musicPlayer.getCurrentTime();

    float minTimeDiff = 9999.0f;
    int bestIdx = -1;
    for (int i = 0; i < (int)m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        if (m_noteRuntimes[i].track != track) continue;
        // skip hold notes that are already being held
        if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
        float timeDiff = std::abs(currentTime - m_noteRuntimes[i].targetTime);
        if (timeDiff < minTimeDiff) { minTimeDiff = timeDiff; bestIdx = i; }
    }
    if (bestIdx < 0) return;  // no eligible note found

    auto& nr = m_noteRuntimes[bestIdx];

    // hold note press logic
    if (nr.type == 1) {
        float timeDiff = std::abs(currentTime - nr.targetTime);
        if (timeDiff > m_goodTimeWindow) return; // too early or too late, ignore
        nr.isHeld = true;
                    static const sf::Color hc2[4] = {sf::Color(0,220,255),sf::Color(255,100,200),sf::Color(255,210,0),sf::Color(100,230,100)};
            const auto& hc3 = hc2[nr.track % 4];
            m_holdBars[bestIdx].setFillColor(sf::Color(hc3.r, hc3.g, hc3.b, 120));
        // score for the successful press
        nr.processed = true; // marks the head hit
        if (timeDiff < m_perfectTimeWindow) {
            m_score += 50; m_perfectCount++;
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("HOLD");
                m_judgmentText->setFillColor(sf::Color::Yellow);
            }
        } else if (timeDiff < m_greatTimeWindow) {
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
        static std::mt19937 rngJh(std::random_device{}());
        static const sf::Color jhc[6] = {
            sf::Color(0, 220, 255), sf::Color(255, 100, 200), sf::Color(255, 210, 0),
            sf::Color(100, 230, 100), sf::Color(255, 150, 50), sf::Color(200, 100, 255)
        };
        m_comboPopColor = jhc[std::uniform_int_distribution<int>(0,5)(rngJh)];
        m_comboPopScale = 1.8f;
        m_comboPopTimer = 0.35f;
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
    if      (minTimeDiff < m_perfectTimeWindow) result = JudgeResult::Perfect;
    else if (minTimeDiff < m_greatTimeWindow)   result = JudgeResult::Great;
    else if (minTimeDiff < m_goodTimeWindow)    result = JudgeResult::Good;
    else if (minTimeDiff < m_missTimeWindow)    result = JudgeResult::Miss;
    else                                        return;  // beyond 300ms

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
            if (m_judgmentText.has_value()) {
                m_judgmentText->setString("BREAK");
                m_judgmentText->setFillColor(sf::Color::Red);
            }
            m_judgmentDisplayTimer = 0.8f;
        }
    }
}

void GameplayScene::onNoteJudged(JudgeResult result) {
    int prevCombo = m_combo;  // track for pop animation
    switch (result) {
    case JudgeResult::Perfect: {
        m_score += 100; m_combo++; m_perfectCount++;
        if (m_judgmentText.has_value()) m_judgmentText->setString("PERFECT");
        m_lastJudgmentColor = sf::Color::Yellow;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 25, sf::Color::Yellow, 60, 250, 0.3f, 0.8f, 2.0f, 7.0f);
        addScorePopup(cx, m_judgmentLineY, " +100", sf::Color::Yellow);
        addHitRing(cx, m_judgmentLineY, sf::Color::Yellow);
        break;
    }
        break;
    case JudgeResult::Great: {
        m_score += 50; m_combo++; m_greatCount++;
        if (m_judgmentText.has_value()) m_judgmentText->setString("GREAT");
        m_lastJudgmentColor = sf::Color::Cyan;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 20, sf::Color::Cyan, 50, 200, 0.3f, 0.7f, 2.0f, 6.0f);
        addScorePopup(cx, m_judgmentLineY, " +50", sf::Color::Cyan);
        addHitRing(cx, m_judgmentLineY, sf::Color::Cyan);
        break;
    }
        break;
    case JudgeResult::Good: {
        if (m_judgmentText.has_value()) m_judgmentText->setString("GOOD");
        m_score += 25; m_combo++; m_goodCount++;
        m_lastJudgmentColor = sf::Color::Green;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 15, sf::Color::Green, 40, 160, 0.3f, 0.6f, 2.0f, 5.0f);
        addScorePopup(cx, m_judgmentLineY, " +25", sf::Color::Green);
        addHitRing(cx, m_judgmentLineY, sf::Color::Green);
        break;
    }
        break;
    case JudgeResult::Miss: {
        if (m_judgmentText.has_value()) m_judgmentText->setString("MISS");
        m_missCount++; m_combo = 0;
        m_lastJudgmentColor = sf::Color::Red;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 8, sf::Color::Red, 30, 100, 0.2f, 0.5f, 1.0f, 4.0f);
        addScorePopup(cx, m_judgmentLineY, " +0", sf::Color::Red);
        addHitRing(cx, m_judgmentLineY, sf::Color::Red);
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

    // score milestone effect (every +1000)
    int curMilestone = m_score / 1000;
    if (curMilestone > m_lastScoreMilestone) {
        m_lastScoreMilestone = curMilestone;
        m_milestoneGlowTimer = 0.5f;
        static std::mt19937 rngMs(std::random_device{}());
        static const sf::Color msColors[6] = {
            sf::Color(0, 220, 255), sf::Color(255, 100, 200), sf::Color(255, 210, 0),
            sf::Color(100, 230, 100), sf::Color(255, 150, 50), sf::Color(200, 100, 255)
        };
        sf::Color mc = msColors[std::uniform_int_distribution<int>(0,5)(rngMs)];
        m_milestoneFX.emit({120.0f, m_judgmentLineY}, 30, mc, 80, 300, 0.4f, 1.2f, 2.0f, 7.0f);
        m_milestoneFX.emit({1160.0f, m_judgmentLineY}, 30, mc, 80, 300, 0.4f, 1.2f, 2.0f, 7.0f);
    }

    // combo pop animation (when combo increases)
    if (m_combo > prevCombo) {
        static std::mt19937 rngCp(std::random_device{}());
        static const sf::Color cpColors[6] = {
            sf::Color(0, 220, 255), sf::Color(255, 100, 200), sf::Color(255, 210, 0),
            sf::Color(100, 230, 100), sf::Color(255, 150, 50), sf::Color(200, 100, 255)
        };
        m_comboPopColor = cpColors[std::uniform_int_distribution<int>(0,5)(rngCp)];
        m_comboPopScale = 1.8f;
        m_comboPopTimer = 0.35f;
    }
}

void GameplayScene::autoMissCheck() {
    float currentTime = m_simTime;
    if (m_musicPlayer.isLoaded())
        currentTime = m_musicPlayer.getCurrentTime();

    for (auto& nr : m_noteRuntimes) {
        if (!nr.active || nr.processed) continue;
        // don't auto-miss notes being held
        if (nr.type == 1 && nr.isHeld) continue;
        if (currentTime > nr.targetTime + m_missTimeWindow && nr.y > m_judgmentLineY) {
            nr.processed = true;
            m_missCount++;
            m_combo = 0;
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






void GameplayScene::addScorePopup(float x, float y, const std::string& text, const sf::Color& color) {
    ScorePopup sp;
    sp.text.emplace(m_font, text, 26);
    sp.text->setFillColor(color);
    auto sb = sp.text->getLocalBounds();
    sp.text->setOrigin({sb.size.x / 2.0f, sb.size.y / 2.0f});
    sp.text->setPosition({x, y - 20});
    sp.life = 0.8f;
    m_scorePopups.push_back(std::move(sp));
}

void GameplayScene::addHitRing(float x, float y, const sf::Color& color) {
    HitRing hr;
    hr.shape.setRadius(8.0f);
    hr.shape.setPosition({x - 8, y - 8});
    hr.shape.setFillColor(sf::Color::Transparent);
    hr.shape.setOutlineThickness(3.0f);
    hr.shape.setOutlineColor(color);
    hr.life = 0.5f;
    m_hitRings.push_back(std::move(hr));
}
