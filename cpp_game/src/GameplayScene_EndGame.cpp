#include "GameplayScene.h"
#include "ResultScene.h"
#include <iostream>

void GameplayScene::endGame() {
    m_songFinished = true;
    m_isPlaying = false;
    m_musicPlayer.stop();

    int total = m_perfectCount + m_greatCount + m_goodCount + m_missCount;
    float accuracy = (total > 0) ? (float)(m_perfectCount + m_greatCount) / total * 100.0f : 0.0f;

    std::string grade;
    if (accuracy >= 90.0f)      grade = "S";
    else if (accuracy >= 75.0f) grade = "A";
    else if (accuracy >= 60.0f) grade = "B";
    else if (accuracy >= 40.0f) grade = "C";
    else                        grade = "D";

    std::cout << "=== ROUND END ===\n"
              << "Score: " << m_score << " | Max Combo: " << m_maxCombo << "\n"
              << "Perfect: " << m_perfectCount << " Great: " << m_greatCount
              << " Good: " << m_goodCount << " Miss: " << m_missCount << "\n"
              << "Accuracy: " << accuracy << "%  Grade: " << grade << "\n";

    requestPop();
}

ResultData GameplayScene::getResultData() const {
    ResultData d;
    d.score = m_score; d.maxCombo = m_maxCombo;
    d.perfectCount = m_perfectCount; d.greatCount = m_greatCount;
    d.goodCount = m_goodCount; d.missCount = m_missCount;
    d.songTitle = m_songInfo.title;
    return d;
}



