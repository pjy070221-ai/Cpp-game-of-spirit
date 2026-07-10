#include "GameplayScene.h"
#include "SettingsData.h"
#include "ResourceManager.h"

std::string GameplayScene::s_chartPath = "";

bool GameplayScene::s_returnToMenu = false;


GameplayScene::GameplayScene() : m_lastJudgmentColor(sf::Color::White)
{
}

void GameplayScene::onEnter() {
    if (!s_chartPath.empty()) {
        loadChart(s_chartPath);
    } else {
        m_beatmapParser.generateExampleBeatmap(4, 30.0f);
        m_noteData = m_beatmapParser.getNotes();
        m_songInfo = m_beatmapParser.getSongInfo();
    }

    applySettings();

    sf::Font* fontPtr = ResourceManager::instance().loadFont("assets/fonts/msyh.ttf");
    if (fontPtr) {
        m_font = *fontPtr;
        m_scoreText.emplace(m_font, "Score: 0", 24);
        m_comboText.emplace(m_font, "", 48);
        m_judgmentText.emplace(m_font, "", 36);
        m_songTitleText.emplace(m_font, m_songInfo.title, 20);
    }

    buildBackground();
    buildTracks();
    buildJudgmentLine();
    startGame();
}

void GameplayScene::onExit() {
    m_musicPlayer.stop();
}

void GameplayScene::loadChart(const std::string& filePath) {
    if (!m_beatmapParser.loadFromFile(filePath))
        m_beatmapParser.generateExampleBeatmap(4, 30.0f);
    m_noteData = m_beatmapParser.getNotes();
    m_songInfo = m_beatmapParser.getSongInfo();
}

void GameplayScene::startGame() {
    m_noteIndex = 0;
    m_score = 0; m_combo = 0; m_maxCombo = 0;
    m_perfectCount = 0; m_greatCount = 0;
    m_goodCount = 0; m_missCount = 0;
    m_noteRuntimes.clear();
    m_activeShapes.clear();
    m_holdBars.clear();
    m_keysHeld[0] = m_keysHeld[1] = m_keysHeld[2] = m_keysHeld[3] = false;
    m_songFinished = false;
    m_isPlaying = true;
    m_musicPlayer.play();
}

void GameplayScene::applySettings() {
    SettingsData s;
    m_noteSpeedPixels = 200.0f + s.getNoteSpeed() * 30.0f;
    m_musicPlayer.setVolume(s.getMasterVolume());
}

void GameplayScene::buildTracks() {
    m_tracks.clear();
    float startX = (m_screenWidth - (m_trackCount * m_trackWidth + (m_trackCount - 1) * m_trackSpacing)) / 2.0f;
    for (int i = 0; i < m_trackCount; ++i) {
        sf::RectangleShape rect({m_trackWidth, m_judgmentLineY - 50.0f});
        rect.setPosition({startX + i * (m_trackWidth + m_trackSpacing), 50.0f});
        rect.setFillColor(sf::Color(30, 20, 50, 80));
        rect.setOutlineThickness(1.0f);
        rect.setOutlineColor(sf::Color(60, 40, 80, 120));
        m_tracks.push_back(rect);
    }
}

void GameplayScene::buildJudgmentLine() {
    m_judgmentLineShape.setSize({m_screenWidth, 3.0f});
    m_judgmentLineShape.setPosition({0.0f, m_judgmentLineY});
    m_judgmentLineShape.setFillColor(sf::Color::Cyan);
}

void GameplayScene::buildBackground() {
    m_bgGradient.resize(4);
    m_bgGradient[0] = sf::Vertex({0.0f, 0.0f}, sf::Color(10, 5, 20));
    m_bgGradient[1] = sf::Vertex({m_screenWidth, 0.0f}, sf::Color(10, 5, 20));
    m_bgGradient[2] = sf::Vertex({0.0f, m_screenHeight}, sf::Color(20, 10, 40));
    m_bgGradient[3] = sf::Vertex({m_screenWidth, m_screenHeight}, sf::Color(20, 10, 40));
}




