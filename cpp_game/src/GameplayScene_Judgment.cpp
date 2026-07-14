#include "GameplayScene.h"
#include <cmath>

void GameplayScene::checkJudgment(int track) {
    // ======== 按键判定：查找当前 track 上最近的未处理音符 ========
    float currentTime = m_simTime;
    if (m_musicPlayer.isLoaded())
        currentTime = m_musicPlayer.getCurrentTime();

    float minTimeDiff = 9999.0f;
    int bestIdx = -1;
    for (int i = 0; i < (int)m_noteRuntimes.size(); ++i) {
        if (!m_noteRuntimes[i].active || m_noteRuntimes[i].processed) continue;
        if (m_noteRuntimes[i].track != track) continue;
        // 跳过已按住的 Hold 音符
        if (m_noteRuntimes[i].type == 1 && m_noteRuntimes[i].isHeld) continue;
        float timeDiff = std::abs(currentTime - m_noteRuntimes[i].targetTime);
        if (timeDiff < minTimeDiff) { minTimeDiff = timeDiff; bestIdx = i; }
    }
    if (bestIdx < 0) return;  // 该 track 无可判定音符（已处理或无活跃）

    auto& nr = m_noteRuntimes[bestIdx];

    // Hold 长按音符头部判定
    if (nr.type == 1) {
        float timeDiff = std::abs(currentTime - nr.targetTime);
        if (timeDiff > m_goodTimeWindow) return; // 过早或过晚，忽略此次按键
        nr.isHeld = true;
                    static const sf::Color hc2[4] = {sf::Color(0,220,255),sf::Color(255,100,200),sf::Color(255,210,0),sf::Color(100,230,100)};
            const auto& hc3 = hc2[nr.track % 4];
            m_holdBars[bestIdx].setFillColor(sf::Color(hc3.r, hc3.g, hc3.b, 120));
        // Hold 头部按下计分
        nr.processed = true; // 标记头部已命中
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
        if (m_combo > m_maxCombo) m_maxCombo = m_combo;
        m_judgmentDisplayTimer = 0.8f;
        if (m_scoreText.has_value())
            m_scoreText->setString("Score: " + std::to_string(m_score));
        if (m_comboText.has_value())
            m_comboText->setString(std::to_string(m_combo));
        // 取消 processed 标记，让 update() 追踪 Hold 尾部完成
        nr.processed = false;
        playHoldSound();
        return;
    }

    m_lastHitTrack = track;
    // 普通音符（Tap）判定
    JudgeResult result;
    if      (minTimeDiff < m_perfectTimeWindow) result = JudgeResult::Perfect;
    else if (minTimeDiff < m_greatTimeWindow)   result = JudgeResult::Great;
    else if (minTimeDiff < m_goodTimeWindow)    result = JudgeResult::Good;
    else if (minTimeDiff < m_missTimeWindow)    result = JudgeResult::Miss;
    else                                        return;  // 超过 miss 窗口，不处理（留给 autoMissCheck）

    onNoteJudged(result);
    nr.processed = true;
    playTapSound();
}

void GameplayScene::checkHoldRelease(int track) {
    for (auto& nr : m_noteRuntimes) {
        if (!nr.active || nr.track != track) continue;
        if (nr.type == 1 && nr.isHeld && !nr.processed) {
            // 提前松手 = BREAK（视为 Miss）
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
        m_score += 25; m_combo++; m_goodCount++; m_hp -= 3;
        if (m_judgmentText.has_value()) m_judgmentText->setString("GOOD");
        m_lastJudgmentColor = sf::Color::Green;
        float cx = getTrackCenterX(m_lastHitTrack);
        m_hitFX.emit({cx, m_judgmentLineY}, 15, sf::Color::Green, 40, 160, 0.3f, 0.6f, 2.0f, 5.0f);
        addScorePopup(cx, m_judgmentLineY, " +25", sf::Color::Green);
        addHitRing(cx, m_judgmentLineY, sf::Color::Green);
        break;
    }
        break;
    case JudgeResult::Miss: {
        m_missCount++; m_combo = 0; m_hp -= 8;
        if (m_judgmentText.has_value()) m_judgmentText->setString("MISS");
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
}

void GameplayScene::autoMissCheck() {
    float currentTime = m_simTime;
    if (m_musicPlayer.isLoaded())
        currentTime = m_musicPlayer.getCurrentTime();

    for (auto& nr : m_noteRuntimes) {
        if (!nr.active || nr.processed) continue;
        // 不自动 Miss 正在被按住的 Hold 音符
        if (nr.type == 1 && nr.isHeld) continue;
        if (currentTime > nr.targetTime + m_missTimeWindow && nr.y > m_judgmentLineY) {
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

void GameplayScene::playTapSound() {
    if (m_tapSound) m_tapSound->play();
}

void GameplayScene::playHoldSound() {
    if (m_holdSound) m_holdSound->play();
}
