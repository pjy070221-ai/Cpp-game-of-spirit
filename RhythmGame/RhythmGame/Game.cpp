#include <cstdint>
#include "Game.h"
#include <random>
#include <fstream>
#include <windows.h>

Game::Game()
    : settings("settings.json")
    , isRunning(true)
    , noteSpeedPixels(400.0f)
    , isMenuOpen(false)
    , menuSelection(0)
    , menuText(font)
    , menuTitle(font)
    , titleText(font)
    , subtitleText(font)
    , speedText(font)
    , infoText(font)
    , comboText(font)
    , judgmentText(font)
    , scoreText(font)
    , pulseTime(0.0f)
    , glowIntensity(0.5f)
    , combo(0)
    , maxCombo(0)
    , score(0)
    , judgmentDisplayTimer(0.0f)
    , judgmentLineY(550.f)
    , trackCount(4)
    , trackWidth(80.f)
    , trackSpacing(20.f)
    , screenWidth(1280.f)
    , screenHeight(720.f)
    , noteIndex(0)
    , isPlaying(false)
    , perfectWindow(40.0f)
    , greatWindow(80.0f)
    , goodWindow(150.0f)
    , lastJudgmentColor(sf::Color::Yellow)
{
    applySettings();

    // 加载字体
    bool fontLoaded = false;
    if (font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        fontLoaded = true;
        std::cout << "[Game] Font loaded: Arial" << std::endl;
    }
    else if (font.openFromFile("C:/Windows/Fonts/simhei.ttf")) {
        fontLoaded = true;
        std::cout << "[Game] Font loaded: SimHei" << std::endl;
    }
    else if (font.openFromFile("C:/Windows/Fonts/simsun.ttc")) {
        fontLoaded = true;
        std::cout << "[Game] Font loaded: SimSun" << std::endl;
    }

    if (!fontLoaded) {
        std::cout << "[Game] Warning: Font load failed" << std::endl;
    }

    if (fontLoaded) {
        updateUI();

        // Combo Display
        comboText.setFont(font);
        comboText.setString("0");
        comboText.setCharacterSize(48);
        comboText.setFillColor(sf::Color(0, 255, 200));
        comboText.setOutlineColor(sf::Color(0, 200, 255));
        comboText.setOutlineThickness(2.f);
        comboText.setPosition(sf::Vector2f(screenWidth - 230.f, 150.f));

        // Score Display
        scoreText.setFont(font);
        scoreText.setString("Score: 0");
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color(255, 200, 100));
        scoreText.setPosition(sf::Vector2f(screenWidth - 250.f, 220.f));

        // Judgment Display
        judgmentText.setFont(font);
        judgmentText.setCharacterSize(64);
        judgmentText.setFillColor(sf::Color::Yellow);
        judgmentText.setPosition(sf::Vector2f(screenWidth / 2 - 120.f, screenHeight / 2 - 100.f));

        // Speed/Volume Display
        speedText.setFont(font);
        speedText.setCharacterSize(18);
        speedText.setFillColor(sf::Color(0, 255, 200, 200));
        speedText.setPosition(sf::Vector2f(20.f, screenHeight - 70.f));
        updateSpeedText();

        // Menu
        menuTitle.setFont(font);
        menuTitle.setString("== SETTINGS MENU ==");
        menuTitle.setCharacterSize(30);
        menuTitle.setFillColor(sf::Color(255, 200, 100));
        menuTitle.setPosition(sf::Vector2f(screenWidth / 2 - 170.f, 120.f));

        menuText.setFont(font);
        menuText.setCharacterSize(24);
        menuText.setFillColor(sf::Color(200, 220, 255));
        menuText.setPosition(sf::Vector2f(screenWidth / 2 - 160.f, 180.f));

        menuOptions = {
            "Volume: " + std::to_string((int)(settings.getMasterVolume() * 100)) + "%",
            "Note Speed: " + std::to_string(settings.getNoteSpeed()),
            "Fullscreen: " + std::string(settings.getFullscreen() ? "ON" : "OFF"),
            "Close Menu"
        };
        updateMenuText();

        // Tracks
        float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
        for (int i = 0; i < trackCount; ++i) {
            sf::RectangleShape track(sf::Vector2f(trackWidth, screenHeight - 200.f));
            track.setFillColor(sf::Color(50, 50, 80, 80));
            track.setOutlineThickness(1.f);
            track.setOutlineColor(sf::Color(0, 255, 255, 100));
            track.setPosition(sf::Vector2f(
                startX + i * (trackWidth + trackSpacing),
                100.f
            ));
            tracks.push_back(track);
        }

        // Judgment Line
        judgmentLine.setSize(sf::Vector2f(
            trackCount * trackWidth + (trackCount - 1) * trackSpacing + 20.f,
            3.f
        ));
        judgmentLine.setFillColor(sf::Color(0, 255, 255));
        judgmentLine.setPosition(sf::Vector2f(
            startX - 10.f,
            judgmentLineY
        ));

        // Particles
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> posDist(0, screenWidth);
        std::uniform_real_distribution<> sizeDist(1, 3);
        std::uniform_int_distribution<> colorDist(0, 2);

        for (int i = 0; i < 30; ++i) {
            sf::CircleShape particle((float)sizeDist(gen));
            particle.setPosition(sf::Vector2f((float)posDist(gen), (float)(posDist(gen) / 2 + 50)));
            particle.setFillColor(
                colorDist(gen) == 0 ? sf::Color(0, 255, 255, 80) :
                colorDist(gen) == 1 ? sf::Color(255, 0, 255, 80) :
                sf::Color(100, 200, 255, 80)
            );
            particles.push_back(particle);
        }

        // ============================================================
        // 加载自定义音乐和谱面
        // ============================================================
        char buffer[256];
        GetCurrentDirectoryA(256, buffer);
        std::cout << "[Game] Current directory: " << buffer << std::endl;

        std::string beatmapFile = "song.json";
        std::cout << "[Game] Looking for: " << beatmapFile << std::endl;

        if (beatmapParser.loadFromFile(beatmapFile)) {
            SongInfo info = beatmapParser.getSongInfo();
            std::cout << "[Game] Loaded beatmap: " << info.title << std::endl;

            if (!info.musicFile.empty()) {
                std::cout << "[Game] Trying to load music: " << info.musicFile << std::endl;
                if (musicPlayer.load(info.musicFile)) {
                    musicPlayer.setOffset(info.offset);
                    musicPlayer.setVolume(settings.getMasterVolume());
                    std::cout << "[Game] Loaded music: " << info.musicFile << std::endl;

                    // ============================================================
                    // ★★★ 修改这个数字来控制音符数量 ★★★
                    // ============================================================
                    int targetNotes = 400;  // ← 改这里！100=简单, 200=较易, 400=适中, 600=困难, 800=专家

                    float bpm = info.bpm;
                    if (bpm <= 0) bpm = 111.0f;

                    float beatDuration = 60.0f / bpm;
                    float interval = beatDuration / 2.0f;  // 8分音符

                    notes.clear();
                    std::random_device rd2;
                    std::mt19937 gen2(rd2());
                    std::uniform_int_distribution<> trackDist2(0, trackCount - 1);

                    for (int i = 0; i < targetNotes; ++i) {
                        NoteData nd;
                        nd.time = i * interval + 0.5f;
                        nd.track = trackDist2(gen2);
                        notes.push_back(nd);
                    }

                    noteIndex = 0;
                    trackCount = info.trackCount;
                    std::cout << "[Game] BPM: " << bpm << std::endl;
                    std::cout << "[Game] Generated " << notes.size() << " notes" << std::endl;

                }
                else {
                    std::cout << "[Game] Failed to load music: " << info.musicFile << std::endl;
                    generateExampleNotes();
                }
            }
            else {
                std::cout << "[Game] No music file specified" << std::endl;
                generateExampleNotes();
            }
        }
        else {
            std::cout << "[Game] Failed to open: " << beatmapFile << std::endl;
            std::cout << "[Game] No beatmap found, generating example notes" << std::endl;
            generateExampleNotes();
        }
    }
}

Game::~Game() {
    settings.saveToFile();
    if (musicPlayer.isLoaded()) {
        musicPlayer.stop();
    }
    std::cout << "[Game] Settings saved" << std::endl;
}

// ============================================================
// 生成示例音符（当没有谱面文件时使用）
// ============================================================
void Game::generateExampleNotes() {
    notes.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> trackDist(0, trackCount - 1);

    float currentTime = 1.0f;
    while (currentTime < 30.0f) {
        int track = trackDist(gen);
        NoteData nd;
        nd.time = currentTime;
        nd.track = track;
        notes.push_back(nd);
        currentTime += 0.3f + (float)(std::rand() % 70) / 100.0f;
    }

    noteIndex = 0;
    std::cout << "[Game] Generated " << notes.size() << " example notes" << std::endl;
}

// ============================================================
// 更新 UI
// ============================================================
void Game::updateUI() {
    titleText.setFont(font);
    titleText.setString("== RHYTHM GAME ==");
    titleText.setCharacterSize(56);
    titleText.setFillColor(sf::Color(0, 255, 255));
    titleText.setOutlineColor(sf::Color(0, 150, 255));
    titleText.setOutlineThickness(2.f);
    titleText.setPosition(sf::Vector2f(screenWidth / 2 - 220.f, 40.f));

    subtitleText.setFont(font);
    subtitleText.setString(">> PRESS D/F/J/K <<");
    subtitleText.setCharacterSize(28);
    subtitleText.setFillColor(sf::Color(255, 100, 255));
    subtitleText.setOutlineColor(sf::Color(200, 50, 255));
    subtitleText.setOutlineThickness(1.f);
    subtitleText.setPosition(sf::Vector2f(screenWidth / 2 - 140.f, 110.f));

    infoText.setFont(font);
    infoText.setString(
        "ESC: Exit  |  M: Menu  |  R: Reset  |  SPACE: Play/Pause\n"
        "UP/DOWN: Volume  |  PGUP/PGDN: Speed"
    );
    infoText.setCharacterSize(16);
    infoText.setFillColor(sf::Color(100, 200, 255, 200));
    infoText.setPosition(sf::Vector2f(20.f, 20.f));
}

// ============================================================
// 主循环
// ============================================================
void Game::run() {
    sf::Clock clock;
    while (window.isOpen() && isRunning) {
        float deltaTime = clock.restart().asSeconds();
        handleEvents();
        update(deltaTime);
        render();
    }
}

// ============================================================
// 事件处理
// ============================================================
void Game::handleEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            isRunning = false;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            screenWidth = (float)resized->size.x;
            screenHeight = (float)resized->size.y;
            updateUI();
            float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
            for (int i = 0; i < trackCount; ++i) {
                tracks[i].setPosition(sf::Vector2f(
                    startX + i * (trackWidth + trackSpacing),
                    100.f
                ));
                tracks[i].setSize(sf::Vector2f(trackWidth, screenHeight - 200.f));
            }
            judgmentLine.setPosition(sf::Vector2f(startX - 10.f, judgmentLineY));
            judgmentLine.setSize(sf::Vector2f(
                trackCount * trackWidth + (trackCount - 1) * trackSpacing + 20.f,
                3.f
            ));
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            sf::Keyboard::Key key = keyPressed->code;

            if (isMenuOpen) {
                if (key == sf::Keyboard::Key::Escape) {
                    isMenuOpen = false;
                }
                else if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W) {
                    navigateMenu(-1);
                }
                else if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S) {
                    navigateMenu(1);
                }
                else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space) {
                    executeMenuSelection();
                }
                return;
            }

            // ===== 游戏控制 =====
            if (key == sf::Keyboard::Key::Escape) {
                window.close();
                isRunning = false;
            }
            else if (key == sf::Keyboard::Key::M) {
                toggleMenu();
            }
            else if (key == sf::Keyboard::Key::R) {
                std::cout << "[Game] Resetting..." << std::endl;
                settings.resetToDefaults();
                applySettings();
                updateSpeedText();
                updateMenuText();
                combo = 0;
                maxCombo = 0;
                score = 0;
                comboText.setString("0");
                comboText.setFillColor(sf::Color(0, 255, 200));
                activeNotes.clear();
                noteIndex = 0;
                std::cout << "[Game] Reset complete" << std::endl;
            }
            else if (key == sf::Keyboard::Key::Space) {
                if (musicPlayer.isLoaded()) {
                    if (musicPlayer.isPlaying()) {
                        musicPlayer.pause();
                        isPlaying = false;
                        std::cout << "[Game] Paused" << std::endl;
                    }
                    else {
                        musicPlayer.play();
                        isPlaying = true;
                        std::cout << "[Game] Playing" << std::endl;
                    }
                }
                else {
                    isPlaying = !isPlaying;
                    std::cout << "[Game] Simulated play: " << (isPlaying ? "ON" : "OFF") << std::endl;
                }
            }
            else if (key == sf::Keyboard::Key::S) {
                std::cout << "\n--- Current Settings ---" << std::endl;
                std::cout << "Volume: " << (int)(settings.getMasterVolume() * 100) << "%" << std::endl;
                std::cout << "Note Speed: " << settings.getNoteSpeed() << std::endl;
                std::cout << "Fullscreen: " << (settings.getFullscreen() ? "ON" : "OFF") << std::endl;
                std::cout << "Score: " << score << "  Combo: " << combo << std::endl;
                std::cout << "------------------------\n" << std::endl;
            }
            else if (key == sf::Keyboard::Key::Up) {
                float vol = std::min(1.0f, settings.getMasterVolume() + 0.05f);
                settings.setMasterVolume(vol);
                settings.saveToFile();
                if (musicPlayer.isLoaded()) musicPlayer.setVolume(vol);
                updateSpeedText();
                updateMenuText();
                std::cout << "[Game] Volume: " << (int)(vol * 100) << "%" << std::endl;
            }
            else if (key == sf::Keyboard::Key::Down) {
                float vol = std::max(0.0f, settings.getMasterVolume() - 0.05f);
                settings.setMasterVolume(vol);
                settings.saveToFile();
                if (musicPlayer.isLoaded()) musicPlayer.setVolume(vol);
                updateSpeedText();
                updateMenuText();
                std::cout << "[Game] Volume: " << (int)(vol * 100) << "%" << std::endl;
            }
            else if (key == sf::Keyboard::Key::PageUp) {
                float speed = std::min(10.0f, settings.getNoteSpeed() + 0.5f);
                settings.setNoteSpeed(speed);
                settings.saveToFile();
                noteSpeedPixels = 200.f + speed * 30.f;
                updateSpeedText();
                updateMenuText();
                std::cout << "[Game] Note Speed: " << speed << std::endl;
            }
            else if (key == sf::Keyboard::Key::PageDown) {
                float speed = std::max(1.0f, settings.getNoteSpeed() - 0.5f);
                settings.setNoteSpeed(speed);
                settings.saveToFile();
                noteSpeedPixels = 200.f + speed * 30.f;
                updateSpeedText();
                updateMenuText();
                std::cout << "[Game] Note Speed: " << speed << std::endl;
            }
            else if (key == sf::Keyboard::Key::F11) {
                settings.setFullscreen(!settings.getFullscreen());
                settings.saveToFile();
                applySettings();
                updateMenuText();
                std::cout << "[Game] Fullscreen: " << (settings.getFullscreen() ? "ON" : "OFF") << std::endl;
            }
            // ===== 音游按键 D/F/J/K =====
            else if (key == sf::Keyboard::Key::D) {
                checkJudgment(0);
            }
            else if (key == sf::Keyboard::Key::F) {
                checkJudgment(1);
            }
            else if (key == sf::Keyboard::Key::J) {
                checkJudgment(2);
            }
            else if (key == sf::Keyboard::Key::K) {
                checkJudgment(3);
            }
        }
    }
}

// ============================================================
// 更新
// ============================================================
void Game::update(float deltaTime) {
    pulseTime += deltaTime;
    glowIntensity = 0.5f + 0.3f * std::sin(pulseTime * 2.0f);

    updateParticles(deltaTime);

    // ===== 音游更新 =====
    float currentTime = 0.0f;
    if (musicPlayer.isLoaded() && musicPlayer.isPlaying()) {
        currentTime = musicPlayer.getCurrentTime();
    }
    else if (isPlaying) {
        static float simTime = 0.0f;
        simTime += deltaTime;
        currentTime = simTime;
    }

    if (isPlaying) {
        spawnNotes(currentTime);
    }

    for (auto it = activeNotes.begin(); it != activeNotes.end(); ) {
        it->move(sf::Vector2f(0.f, noteSpeedPixels * deltaTime));

        if (it->getPosition().y > screenHeight + 100.f) {
            onNoteJudged(JudgeResult::Miss, 0);
            it = activeNotes.erase(it);
        }
        else {
            ++it;
        }
    }

    if (judgmentDisplayTimer > 0) {
        judgmentDisplayTimer -= deltaTime;
        float alpha = std::min(1.0f, judgmentDisplayTimer * 4.0f);
        sf::Color color = lastJudgmentColor;
        color.a = (uint8_t)(alpha * 255);
        judgmentText.setFillColor(color);
    }
}

// ============================================================
// 生成音符
// ============================================================
void Game::spawnNotes(float currentTime) {
    while (noteIndex < (int)notes.size() && notes[noteIndex].time <= currentTime + 1.5f) {
        if (notes[noteIndex].time >= currentTime) {
            sf::CircleShape note(22.f);
            note.setFillColor(sf::Color(0, 255, 255, 220));
            note.setOutlineThickness(2.f);
            note.setOutlineColor(sf::Color(0, 200, 255));
            note.setOrigin(sf::Vector2f(22.f, 22.f));

            float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
            float x = startX + notes[noteIndex].track * (trackWidth + trackSpacing) + trackWidth / 2.f;
            float y = 100.f;

            note.setPosition(sf::Vector2f(x, y));
            activeNotes.push_back(note);
        }
        noteIndex++;
    }
}

// ============================================================
// 判定
// ============================================================
void Game::checkJudgment(int track) {
    if (!isPlaying) return;

    float minDist = 9999.0f;
    int bestIdx = -1;

    for (int i = 0; i < (int)activeNotes.size(); ++i) {
        float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
        float trackX = startX + track * (trackWidth + trackSpacing) + trackWidth / 2.f;
        float noteX = activeNotes[i].getPosition().x;

        if (std::abs(noteX - trackX) < trackWidth / 2.f) {
            float dist = std::abs(activeNotes[i].getPosition().y - judgmentLineY);
            if (dist < minDist) {
                minDist = dist;
                bestIdx = i;
            }
        }
    }

    if (bestIdx >= 0) {
        JudgeResult result;
        if (minDist < 20.f) {
            result = JudgeResult::Perfect;
        }
        else if (minDist < 50.f) {
            result = JudgeResult::Great;
        }
        else if (minDist < 100.f) {
            result = JudgeResult::Good;
        }
        else {
            result = JudgeResult::Miss;
        }

        activeNotes.erase(activeNotes.begin() + bestIdx);
        onNoteJudged(result, track);
    }
}

// ============================================================
// 判定结果处理
// ============================================================
void Game::onNoteJudged(JudgeResult result, int track) {
    std::string text;
    sf::Color color;
    int points = 0;

    switch (result) {
    case JudgeResult::Perfect:
        text = "PERFECT";
        color = sf::Color::Yellow;
        points = 100;
        combo++;
        break;
    case JudgeResult::Great:
        text = "GREAT";
        color = sf::Color::Cyan;
        points = 50;
        combo++;
        break;
    case JudgeResult::Good:
        text = "GOOD";
        color = sf::Color::Green;
        points = 25;
        combo++;
        break;
    case JudgeResult::Miss:
        text = "MISS";
        color = sf::Color::Red;
        points = 0;
        combo = 0;
        break;
    default:
        return;
    }

    score += points;
    if (combo > maxCombo) maxCombo = combo;

    judgmentText.setString(text);
    judgmentText.setFillColor(color);
    judgmentDisplayTimer = 0.8f;
    lastJudgmentColor = color;

    comboText.setString(std::to_string(combo));
    if (combo >= 50) {
        comboText.setFillColor(sf::Color(255, 200, 0));
    }
    else if (combo >= 20) {
        comboText.setFillColor(sf::Color(255, 100, 255));
    }
    else {
        comboText.setFillColor(sf::Color(0, 255, 200));
    }

    scoreText.setString("Score: " + std::to_string(score));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> angleDist(0, 6.28);
    std::uniform_real_distribution<> speedDist(50, 200);

    float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
    float x = startX + track * (trackWidth + trackSpacing) + trackWidth / 2.f;

    for (int i = 0; i < 25; ++i) {
        sf::CircleShape particle(3.f);
        float angle = (float)angleDist(gen);
        float speed = (float)speedDist(gen);
        particle.setPosition(sf::Vector2f(x, judgmentLineY));
        particle.setFillColor(color);
        particles.push_back(particle);
    }
}

// ============================================================
// 渲染
// ============================================================
void Game::render() {
    window.clear(sf::Color(10, 5, 20));

    drawNeonBackground();
    drawNoteTracks();
    drawJudgmentLine();
    drawNeonTitle();

    for (const auto& note : activeNotes) {
        window.draw(note);
    }

    if (font.getInfo().family != "") {
        window.draw(infoText);
        window.draw(speedText);
        window.draw(comboText);
        window.draw(scoreText);

        if (judgmentDisplayTimer > 0) {
            window.draw(judgmentText);
        }

        if (isMenuOpen) {
            sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(overlay);

            sf::RectangleShape menuBg(sf::Vector2f(420.f, 350.f));
            menuBg.setFillColor(sf::Color(20, 10, 40, 230));
            menuBg.setOutlineThickness(2.f);
            menuBg.setOutlineColor(sf::Color(0, 255, 255));
            menuBg.setPosition(sf::Vector2f(screenWidth / 2 - 210.f, 100.f));
            window.draw(menuBg);

            window.draw(menuTitle);
            window.draw(menuText);
        }
    }

    for (const auto& particle : particles) {
        window.draw(particle);
    }

    window.display();
}

// ============================================================
// 粒子更新
// ============================================================
void Game::updateParticles(float deltaTime) {
    for (auto& particle : particles) {
        particle.move(sf::Vector2f(
            (std::rand() % 100 - 50) * deltaTime,
            -80.f * deltaTime
        ));
        sf::Color color = particle.getFillColor();
        if (color.a > 5) {
            color.a -= 4;
            particle.setFillColor(color);
        }
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const sf::CircleShape& p) {
                return p.getFillColor().a < 5;
            }),
        particles.end()
    );

    if (particles.size() > 150) {
        particles.erase(particles.begin(), particles.begin() + 30);
    }
}

// ============================================================
// 绘制函数
// ============================================================
void Game::drawNeonBackground() {
    sf::CircleShape glow1(300.f);
    glow1.setFillColor(sf::Color(0, 50, 150, 20));
    glow1.setPosition(sf::Vector2f(200.f, 200.f));
    window.draw(glow1);

    sf::CircleShape glow2(400.f);
    glow2.setFillColor(sf::Color(150, 0, 150, 15));
    glow2.setPosition(sf::Vector2f(screenWidth - 400.f, 300.f));
    window.draw(glow2);

    sf::CircleShape glow3(200.f);
    glow3.setFillColor(sf::Color(0, 200, 255, 10));
    glow3.setPosition(sf::Vector2f(screenWidth / 2, 100.f));
    window.draw(glow3);
}

void Game::drawNoteTracks() {
    for (const auto& track : tracks) {
        window.draw(track);
    }

    float startX = (screenWidth - (trackCount * trackWidth + (trackCount - 1) * trackSpacing)) / 2.f;
    for (int i = 0; i < trackCount - 1; ++i) {
        sf::RectangleShape line(sf::Vector2f(1.f, screenHeight - 200.f));
        line.setFillColor(sf::Color(0, 255, 255, 50));
        line.setPosition(sf::Vector2f(
            startX + (i + 1) * (trackWidth + trackSpacing) - trackSpacing / 2.f,
            100.f
        ));
        window.draw(line);
    }
}

void Game::drawJudgmentLine() {
    sf::RectangleShape glow(sf::Vector2f(
        judgmentLine.getSize().x + 40.f,
        judgmentLine.getSize().y + 20.f
    ));
    glow.setFillColor(sf::Color(0, 255, 255, 20));
    glow.setPosition(sf::Vector2f(
        judgmentLine.getPosition().x - 20.f,
        judgmentLine.getPosition().y - 10.f
    ));
    window.draw(glow);

    window.draw(judgmentLine);

    if (static_cast<int>(pulseTime * 4) % 2 == 0) {
        sf::RectangleShape flash(sf::Vector2f(
            judgmentLine.getSize().x,
            2.f
        ));
        flash.setFillColor(sf::Color(0, 255, 255, 100));
        flash.setPosition(judgmentLine.getPosition());
        window.draw(flash);
    }
}

void Game::drawNeonTitle() {
    sf::Text glowTitle = titleText;
    glowTitle.setFillColor(sf::Color(0, 255, 255, 30));
    glowTitle.setPosition(sf::Vector2f(
        titleText.getPosition().x + 5.f,
        titleText.getPosition().y + 5.f
    ));
    window.draw(glowTitle);

    window.draw(titleText);
    window.draw(subtitleText);
}

// ============================================================
// 应用设置
// ============================================================
void Game::applySettings() {
    sf::Vector2u size = settings.getWindowSize();
    screenWidth = (float)size.x;
    screenHeight = (float)size.y;

    unsigned int style = settings.getFullscreen() ? 1 : 0;

    window.create(sf::VideoMode(size), "Rhythm Game", style);
    window.setFramerateLimit(settings.getFpsLimit());
    window.setVerticalSyncEnabled(settings.getVsync());

    noteSpeedPixels = 200.f + settings.getNoteSpeed() * 30.f;

    if (musicPlayer.isLoaded()) {
        musicPlayer.setVolume(settings.getMasterVolume());
    }

    std::cout << "[Game] Settings applied" << std::endl;
}

// ============================================================
// 更新速度文字
// ============================================================
void Game::updateSpeedText() {
    if (font.getInfo().family == "") return;

    speedText.setString(
        "Speed: " + std::to_string(settings.getNoteSpeed()) +
        "  |  Volume: " + std::to_string((int)(settings.getMasterVolume() * 100)) + "%" +
        "  |  Score: " + std::to_string(score) +
        "  |  Combo: " + std::to_string(combo)
    );
}

// ============================================================
// 菜单函数
// ============================================================
void Game::toggleMenu() {
    isMenuOpen = !isMenuOpen;
    if (isMenuOpen) {
        updateMenuText();
    }
}

void Game::updateMenuText() {
    if (font.getInfo().family == "") return;

    menuOptions[0] = "Volume: " + std::to_string((int)(settings.getMasterVolume() * 100)) + "%";
    menuOptions[1] = "Note Speed: " + std::to_string(settings.getNoteSpeed());
    menuOptions[2] = std::string("Fullscreen: ") + (settings.getFullscreen() ? "ON" : "OFF");

    std::string text = "";
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        if (i == static_cast<size_t>(menuSelection)) {
            text += "> " + menuOptions[i] + " <\n";
        }
        else {
            text += "  " + menuOptions[i] + "\n";
        }
    }
    text += "\n+---------------------+";
    text += "\n|  UP/DOWN: Navigate  |";
    text += "\n|  ENTER: Select      |";
    text += "\n|  ESC: Close         |";
    text += "\n+---------------------+";
    menuText.setString(text);
}

void Game::navigateMenu(int direction) {
    menuSelection = (menuSelection + direction + static_cast<int>(menuOptions.size())) % static_cast<int>(menuOptions.size());
    updateMenuText();
}

void Game::executeMenuSelection() {
    switch (menuSelection) {
    case 0: {
        float newVol = std::min(1.0f, settings.getMasterVolume() + 0.1f);
        settings.setMasterVolume(newVol);
        settings.saveToFile();
        if (musicPlayer.isLoaded()) musicPlayer.setVolume(newVol);
        updateSpeedText();
        menuOptions[0] = "Volume: " + std::to_string((int)(newVol * 100)) + "%";
        updateMenuText();
        std::cout << "[Game] Volume: " << (int)(newVol * 100) << "%" << std::endl;
        break;
    }
    case 1: {
        float newSpeed = std::min(10.0f, settings.getNoteSpeed() + 0.5f);
        settings.setNoteSpeed(newSpeed);
        settings.saveToFile();
        noteSpeedPixels = 200.f + newSpeed * 30.f;
        updateSpeedText();
        menuOptions[1] = "Note Speed: " + std::to_string(newSpeed);
        updateMenuText();
        std::cout << "[Game] Note Speed: " << newSpeed << std::endl;
        break;
    }
    case 2: {
        settings.setFullscreen(!settings.getFullscreen());
        settings.saveToFile();
        applySettings();
        menuOptions[2] = std::string("Fullscreen: ") + (settings.getFullscreen() ? "ON" : "OFF");
        updateMenuText();
        std::cout << "[Game] Fullscreen: " << (settings.getFullscreen() ? "ON" : "OFF") << std::endl;
        break;
    }
    case 3:
        isMenuOpen = false;
        break;
    }
}