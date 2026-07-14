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
    // 从 PauseScene 返回主菜单 → 跳过本场景初始化
    if (s_returnToMenu) {
        m_countdownState = CountdownState::None;
        m_isPlaying = false;
        return;
    }

    // 重试标志检测
    if (s_retry) {
        m_countdownState = CountdownState::None;
        m_isPlaying = false;
        return;
    }
    // 倒计时 3-2-1 状态初始化
    m_countdownState = CountdownState::Counting;
    m_countdownTimer = 3.0f;
    if (m_initialized) { m_isPlaying = false; return; } // 重入保护：已初始化则跳过（由静态标志驱动）
    
    m_initialized = true;
    m_countdownShouldStart = true;

    if (!s_chartPath.empty()) {
        loadChart(s_chartPath);
    } else {
        m_beatmapParser.generateExampleBeatmap(4, 30.0f);
        m_noteData = m_beatmapParser.getNotes();
        m_songInfo = m_beatmapParser.getSongInfo();
    }

    // 随机模式：将每个音符随机分配到 4 个轨道
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

    // 加载打击音效（失败不阻塞，静默跳过）
    if (m_tapBuffer.loadFromFile("assets/tap_hit.wav"))
        m_tapSound = new sf::Sound(m_tapBuffer);
    if (m_holdBuffer.loadFromFile("assets/hold_hit.wav"))
        m_holdSound = new sf::Sound(m_holdBuffer);

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
    // startGame(); // 由 Countdown 倒计时结束后自动触发播放
    startGame();
}

void GameplayScene::onExit() {
    m_musicPlayer.stop();
    delete m_tapSound;   m_tapSound = nullptr;
    delete m_holdSound;  m_holdSound = nullptr;
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
    // 播放由倒计时处理器触发
    // m_isPlaying = true;  // 原直接播放，现由倒计时触发
    // m_musicPlayer.play();
}

void GameplayScene::applySettings() {
    SettingsData s;
    m_noteSpeedPixels = 400.0f + s.getNoteSpeed() * 80.0f;  // 流速公式：400 + ns×80（ns=8 时 = 1040px/s）
    m_musicPlayer.setVolume(s.getMasterVolume());
    m_musicPlayer.setOffset(s.getOffset());
    m_autoPlay = s.getAutoPlay();
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













