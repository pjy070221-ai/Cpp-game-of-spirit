#include "GameplayScene.h"
#include "SettingsData.h"
#include "ResourceManager.h"
#include <algorithm>
#include <random>

std::string GameplayScene::s_chartPath = "";

bool GameplayScene::s_retry = false;
bool GameplayScene::s_randomMode = false;
bool GameplayScene::s_returnToMenu = false;


GameplayScene::GameplayScene() : m_lastJudgmentColor(sf::Color::White)
{
}

void GameplayScene::onEnter() {
    // Return to Menu闁挎稒鐭粭澶愬触椤栨艾袟闁稿﹥甯熼鎼佸籍鐠佸湱绀夐柣鈺佺摠鐢瓨娼婚弬鎸庣闁兼寧绮屽畷?
    if (s_returnToMenu) {
        m_countdownState = CountdownState::None;
        m_isPlaying = false;
        return;
    }

    // Retry
    if (s_retry) {
        m_countdownState = CountdownState::None;
        m_isPlaying = false;
        return;
    }
    // 闁煎搫鍊搁〃鏃€寰勮缁椻偓 闁稿﹥甯熼鎼佸籍?3-2-1
    m_countdownState = CountdownState::Counting;
    m_countdownTimer = 3.0f;
    if (m_initialized) { m_isPlaying = false; return; } // 闁哄棗鍊告禒鐘诲箒閵忕媭妲?闁?鐎殿喒鍋撳┑顔碱儐閺屽﹪宕愰幒鏇ㄥ悁闁?
    
    m_initialized = true;
    m_countdownShouldStart = true;

    if (!s_chartPath.empty()) {
        loadChart(s_chartPath);
    } else {
        m_beatmapParser.generateExampleBeatmap(4, 30.0f);
        m_noteData = m_beatmapParser.getNotes();
        m_songInfo = m_beatmapParser.getSongInfo();
    }

    // 闁冲厜鍋撻柍鍏夊亾 闂傚懎绻戝┃鈧俊顖椻偓宕囩闁挎稒宀搁崳鎼佸箳閹烘鍙剧紒妤嬬畳瀵ゆ椽鏌?闁冲厜鍋撻柍鍏夊亾
    if (s_randomMode) {
        s_randomMode = false;
        std::mt19937 rng(std::random_device{}());
        for (auto& note : m_noteData)
            note.track = std::uniform_int_distribution<int>(0, 3)(rng);
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

    // 异象测试事件 — 已禁用
    // std::vector<AnomalyEvent> testEvents = {
    //     { 3.0f, 1.5f, AnomalyType::Flash,           {{"color_r", 1.0f}, {"color_g", 1.0f}, {"color_b", 1.0f}} },
    //     { 6.0f, 2.0f, AnomalyType::ScreenShake,     {{"intensity", 1.0f}} },
    //     { 9.0f, 5.0f, AnomalyType::NoteSpeedChange, {{"speed", 1.8f}} },
    //     {16.0f, 1.5f, AnomalyType::Flash,           {{"color_r", 1.0f}, {"color_g", 0.2f}, {"color_b", 0.2f}} },
    //     {20.0f, 2.5f, AnomalyType::ScreenShake,     {{"intensity", 0.6f}} },
    //     {25.0f, 3.0f, AnomalyType::NoteSpeedChange, {{"speed", 0.4f}} },
    // };
    // m_anomalySystem.setEvents(testEvents);

    m_flashOverlay.setSize({m_screenWidth, m_screenHeight});
    m_flashOverlay.setFillColor(sf::Color::Transparent);
    m_comboFlashOverlay.setSize({m_screenWidth, m_screenHeight});
    m_comboFlashOverlay.setFillColor(sf::Color::Transparent);
    // startGame(); // 闁衡偓闁稖绀嬮柛锔哄妼閳ь剚甯熼鎼佸籍閸撲胶娉㈤柡澶屽枎閹鎮?Countdown 閻犲鍟伴弫?
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
    // 用户 Settings offset 用于微调延迟
    if (!m_songInfo.musicFile.empty()) {
        m_musicPlayer.load(m_songInfo.musicFile);
        SettingsData s;
        m_musicPlayer.setOffset(s.getOffset());
    }
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
    m_hp = 999999;
    m_maxHp = 999999;
    m_simTime = 0.0f;
    m_songFinished = false;
    // play handled by countdown handler
    // m_isPlaying = true;
    // m_musicPlayer.play();
}

void GameplayScene::applySettings() {
    SettingsData s;
    m_noteSpeedPixels = 400.0f + s.getNoteSpeed() * 80.0f;  // ns=8 → 1040px/s
    m_musicPlayer.setVolume(s.getMasterVolume());
    m_musicPlayer.setOffset(s.getOffset());
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













