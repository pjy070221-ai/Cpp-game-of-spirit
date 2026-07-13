#pragma once

#include "IScene.h"
#include "Types.h"
#include "BeatmapParser.h"
#include "MusicPlayer.h"
#include "ParticleSystem.h"
#include "AnomalySystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <optional>

// 闁煎搫鍊搁〃鏃€寰勮缁椻偓闁瑰灚鎸搁崵顕€鎮х憴鍕珡缂備焦鎸婚悗顖涙媴?
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
    static bool s_randomMode;  // set by PackScene 闁?randomize tracks on enter
    static bool s_retry;        // set by PauseScene/ResultScene 闁?restart current song
    static bool s_returnToMenu;       // set by PauseScene -> checked in update
        ~GameplayScene() override = default;
  

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

    ResultData getResultData() const;
    
    // chart path for external loading
    

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

    // chart & audio
    BeatmapParser m_beatmapParser;
    MusicPlayer   m_musicPlayer;
    std::vector<NoteData> m_noteData;
    SongInfo      m_songInfo;
    int  m_noteIndex = 0;
    bool m_isPlaying = false;
    bool m_songFinished = false;
    bool m_initialized = false;   // true after onEnter() runs once
    float m_simTime = 0.0f;       // 婵☆垪鍓濈€氭瑩寮崼鏇熷闁挎稑鐗婂Λ銈夋閸忓懐顔囬柡鍐ㄥ閺併倝鏁?

    // 闁煎搫鍊搁〃鏃€寰勮缁椻偓 闁稿﹥甯熼鎼佸籍?3-2-1
    enum class CountdownState { None, Counting, Started };
    CountdownState m_countdownState = CountdownState::None;
    float m_countdownTimer = 3.0f;
    std::optional<sf::Text> m_cdNumText;
    std::optional<sf::Text> m_cdGoText;
    int m_cdLastDisplayed = -1;
    bool m_countdownShouldStart = false;
    static constexpr float SPAWN_LOOKAHEAD = 1.5f;

    // runtime notes
    std::vector<NoteRuntime> m_noteRuntimes;
    std::vector<sf::RectangleShape> m_activeShapes;  // 闁煎搫鍊搁〃鏃€寰勮缁椻偓 闁活厸鏅涢懜浼存瀹曞浂鍎?
    std::vector<sf::RectangleShape> m_holdBars;
    bool m_keysHeld[4] = {false, false, false, false};

    // judgment 闁?闁哄啫鐖煎Λ鍧楀礆閵堝懐鏆扮紒鎰殔瑜版盯鏁嶉崼銏╂健闁?
    float m_perfectTimeWindow = 0.040f;  // 閸?0ms
    float m_greatTimeWindow   = 0.100f;  // 閸?00ms
    float m_goodTimeWindow    = 0.200f;  // 閸?00ms
    float m_missTimeWindow    = 0.400f;  // 閻℃帒鎳庨崵顓熺▔瀹ュ懏鎯欓幖?
    float m_judgmentLineY = 550.0f;

    // scoring
    int m_score = 0, m_combo = 0, m_maxCombo = 0;
    int m_perfectCount = 0, m_greatCount = 0;
    int m_goodCount = 0, m_missCount = 0;
    int m_hp = 100, m_maxHp = 100;    // 闁煎搫鍊搁〃鏃€寰勮缁椻偓 闁稿鍎遍幃宥夊磹閼测晠鍏囩紓?

    // track layout
    int   m_trackCount = 4;
    float m_trackWidth = 80.0f;
    float m_trackSpacing = 20.0f;
    float m_screenWidth = 1280.0f;
    float m_screenHeight = 720.0f;
    float m_noteSpeedPixels = 400.0f;

    // rendering
    std::vector<sf::RectangleShape> m_tracks;
    sf::RectangleShape m_judgmentLineShape;
    ParticleSystem m_hitFX;
    std::vector<ScorePopup> m_scorePopups;
    std::vector<HitRing> m_hitRings;
    float m_comboFlashTimer = 0.0f;
    sf::RectangleShape m_comboFlashOverlay;
    

    int m_lastHitTrack = 0; // track for particle emit position
    sf::VertexArray m_bgGradient{ sf::PrimitiveType::TriangleStrip };
    // HUD
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
    
    
    float m_glowIntensity = 0.5f;
};











