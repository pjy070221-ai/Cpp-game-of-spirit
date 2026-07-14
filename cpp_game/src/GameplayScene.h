#pragma once

#include "IScene.h"
#include "Types.h"
#include "BeatmapParser.h"
#include "MusicPlayer.h"
#include "ParticleSystem.h"
#include "AnomalySystem.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <memory>
#include <optional>

// 得分弹出文本
struct ScorePopup {
    std::optional<sf::Text> text;
    float life = 1.0f;
};
struct HitRing {
    sf::CircleShape shape;
    float life = 0.5f;
};

struct ResultData {
    int score = 0;
    int maxCombo = 0;
    int perfectCount = 0;
    int greatCount = 0;
    int goodCount = 0;
    int missCount = 0;
    std::string songTitle;
};

class GameplayScene : public IScene {
public:
    GameplayScene();
    static std::string s_chartPath;   // set by PackScene before push
    static bool s_randomMode;  // set by PackScene randomize tracks on enter tracks on enter
    static bool s_retry;        // set by PauseScene/ResultScene restart current song current song
    static bool s_returnToMenu;       // set by PauseScene -> checked in update
    static bool s_giveUp;           // set by PauseScene -> end game immediately
        ~GameplayScene() override = default;
  

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

    ResultData getResultData() const;
    
    // 谱面路径（供外部加载）
    

private:
    void loadChart(const std::string& filePath);
    void startGame();
    void applySettings();
    void buildTracks();
    void buildJudgmentLine();
    void buildBackground();

    void spawnNotes(float currentTime, float effSpeed);
    void checkJudgment(int track);
    void onNoteJudged(JudgeResult result);
    void checkHoldRelease(int track);
    void autoMissCheck();
    bool allNotesProcessed() const;
    void endGame();

    float getTrackCenterX(int track) const;
    void addScorePopup(float x, float y, const std::string& text, const sf::Color& color);
    void addHitRing(float x, float y, const sf::Color& color);
    void playTapSound();
    void playHoldSound();

    // 谱面 & 音频
    BeatmapParser m_beatmapParser;
    MusicPlayer   m_musicPlayer;
    std::vector<NoteData> m_noteData;
    SongInfo      m_songInfo;
    int  m_noteIndex = 0;
    bool m_isPlaying = false;
    bool m_songFinished = false;
    bool m_initialized = false;   // onEnter() 首次执行后置 true
    float m_simTime = 0.0f;       // 模拟计时器（音乐未加载时作为后备时间源）

    // 倒计时状态（3-2-1 开场）
    enum class CountdownState { None, Counting, Started };
    CountdownState m_countdownState = CountdownState::None;
    float m_countdownTimer = 3.0f;
    std::optional<sf::Text> m_cdNumText;
    std::optional<sf::Text> m_cdGoText;
    int m_cdLastDisplayed = -1;
    bool m_countdownShouldStart = false;
    static constexpr float SPAWN_LOOKAHEAD = 3.0f;

    // 运行时音符数据
    std::vector<NoteRuntime> m_noteRuntimes;
    std::vector<sf::RectangleShape> m_activeShapes;  // 活跃音符图形（与 m_noteRuntimes 同步）
    std::vector<sf::RectangleShape> m_holdBars;
    bool m_keysHeld[4] = {false, false, false, false};

    // 判定时间窗口（时间判定，非像素距离）
    float m_perfectTimeWindow = 0.040f;  // 40ms
    float m_greatTimeWindow   = 0.100f;  // 100ms
    float m_goodTimeWindow    = 0.200f;  // 200ms
    float m_missTimeWindow    = 0.400f;  // 自动 Miss 阈值（超出视为漏键）
    float m_judgmentLineY = 550.0f;

    // 计分系统
    int m_score = 0, m_combo = 0, m_maxCombo = 0;
    int m_perfectCount = 0, m_greatCount = 0;
    int m_goodCount = 0, m_missCount = 0;
    int m_hp = 100, m_maxHp = 100;    // HP 血量（归零则歌曲失败）
    bool m_autoPlay = false;           // 自动演奏模式

    // 轨道布局
    int   m_trackCount = 4;
    float m_trackWidth = 80.0f;
    float m_trackSpacing = 20.0f;
    float m_screenWidth = 1280.0f;
    float m_screenHeight = 720.0f;
    float m_noteSpeedPixels = 400.0f;

    // 渲染资源
    std::vector<sf::RectangleShape> m_tracks;
    sf::RectangleShape m_judgmentLineShape;
    ParticleSystem m_hitFX;
    std::vector<ScorePopup> m_scorePopups;
    std::vector<HitRing> m_hitRings;
    float m_comboFlashTimer = 0.0f;
    sf::RectangleShape m_comboFlashOverlay;
    

    // score milestone effect (every +1000)
    int m_lastScoreMilestone = 0;
    float m_milestoneGlowTimer = 0.0f;
    sf::RectangleShape m_milestoneGlowOverlay;
    ParticleSystem m_milestoneFX;

    // combo pop animation
    float m_comboPopScale = 1.0f;
    float m_comboPopTimer = 0.0f;
    sf::Color m_comboPopColor;

    // combo milestone effect (every +5)
    int m_lastComboMilestone = 0;
    float m_comboRingTimer = 0.0f;
    ParticleSystem m_comboFX;

    // score milestone multi-burst
    int m_milestoneBurstCount = 0;
    float m_milestoneBurstTimer = 0.0f;


    int m_lastHitTrack = 0; // track for particle emit position
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    // HUD 文字
    sf::Font m_font;
    std::optional<sf::Text> m_scoreText;
    std::optional<sf::Text> m_comboText;
    std::optional<sf::Text> m_judgmentText;
    std::optional<sf::Text> m_songTitleText;
    float m_judgmentDisplayTimer = 0.0f;
    sf::Color m_lastJudgmentColor;
    float m_pulseTime = 0.0f;
    AnomalySystem m_anomalySystem;
    sf::RectangleShape m_flashOverlay;

    // 打击音效
    sf::SoundBuffer m_tapBuffer;
    sf::SoundBuffer m_holdBuffer;
    sf::Sound* m_tapSound = nullptr;
    sf::Sound* m_holdSound = nullptr;

    float m_glowIntensity = 0.5f;
};











