#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "SettingsData.h"
#include "MusicPlayer.h"
#include "BeatmapParser.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <map>

// ============================================================
// 判定结果枚举
// ============================================================
enum class JudgeResult {
    Perfect,
    Great,
    Good,
    Miss,
    None
};

// ============================================================
// Game 类
// ============================================================
class Game {
public:
    Game();
    ~Game();
    void run();

private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void applySettings();
    void updateSpeedText();
    void updateUI();

    void toggleMenu();
    void navigateMenu(int direction);
    void executeMenuSelection();
    void updateMenuText();

    void updateParticles(float deltaTime);
    void drawNeonBackground();
    void drawJudgmentLine();
    void drawNoteTracks();
    void drawNeonTitle();

    // ===== 音游核心功能 =====
    void generateExampleNotes();
    void spawnNotes(float currentTime);
    void checkJudgment(int track);
    void onNoteJudged(JudgeResult result, int track);
    void showJudgment(const std::string& text, sf::Color color);

    sf::RenderWindow window;
    SettingsData settings;
    bool isRunning;

    std::vector<sf::RectangleShape> tracks;
    sf::RectangleShape judgmentLine;

    sf::Font font;
    sf::Text infoText;
    sf::Text speedText;
    sf::Text titleText;
    sf::Text subtitleText;
    sf::Text comboText;
    sf::Text judgmentText;
    sf::Text scoreText;

    bool isMenuOpen;
    int menuSelection;
    std::vector<std::string> menuOptions;
    sf::Text menuText;
    sf::Text menuTitle;

    std::vector<sf::CircleShape> particles;
    float pulseTime;
    float glowIntensity;

    int combo;
    int maxCombo;
    int score;
    float judgmentDisplayTimer;

    float judgmentLineY;
    int trackCount;
    float trackWidth;
    float trackSpacing;

    float screenWidth;
    float screenHeight;
    float noteSpeedPixels;

    // ===== 音乐和谱面 =====
    MusicPlayer musicPlayer;
    BeatmapParser beatmapParser;
    std::vector<NoteData> notes;
    std::vector<sf::CircleShape> activeNotes;
    int noteIndex;
    bool isPlaying;

    float perfectWindow;
    float greatWindow;
    float goodWindow;
    sf::Color lastJudgmentColor;
};