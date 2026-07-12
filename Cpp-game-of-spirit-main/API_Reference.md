# API Reference — Cross Beat

> 本文件列出所有已实现类的接口签名，用于写新代码时快速查阅，防止接口混用。

---

## Application

**路径：** src/Application.h

```
Application(const sf::String& title, unsigned int width, unsigned int height);
void run();
sf::RenderWindow& getWindow();
SceneManager& getSceneManager();
float getDeltaTime() const;
```

---

## IScene (abstract base)

**路径：** src/IScene.h

```
virtual void onEnter();
virtual void onExit();
virtual void handleEvent(const sf::Event& event);
virtual void update(float dt) = 0;
virtual void render(sf::RenderTarget& target) = 0;
void requestPush(std::unique_ptr<IScene> scene);
void requestPop();
void requestReplace(std::unique_ptr<IScene> scene);
void setSceneManager(SceneManager* manager);
SceneManager* getSceneManager() const;
```

---

## SceneManager

**路径：** src/SceneManager.h

```
void pushScene(std::unique_ptr<IScene> scene);
void popScene();
void replaceScene(std::unique_ptr<IScene> scene);
void handleEvent(const sf::Event& event);
void update(float dt);
void render(sf::RenderTarget& target);
bool isEmpty() const;
```

---

## Types.h

**路径：** src/Types.h

```
enum class JudgeResult { Perfect, Great, Good, Miss, None };

struct NoteData {
    float time; int track; int type;  // 0=normal, 1=hold
    NoteData() = default;
    NoteData(float t, int tr);
};

struct SongInfo {
    string title, artist, musicFile;
    float bpm = 120.f, offset = 0.f;
    int trackCount = 4;
};

struct NoteRuntime {
    int track, type;
    float targetTime, y, noteSpeed;
    bool processed, active;
};
```

---

## BeatmapParser

**路径：** src/BeatmapParser.h

```
bool loadFromFile(const std::string& filePath);
const std::vector<NoteData>& getNotes() const;
const SongInfo& getSongInfo() const;
int getTrackCount() const;
bool isLoaded() const;
void generateExampleBeatmap(int trackCount, float durationSec);
```

---

## MusicPlayer

**路径：** src/MusicPlayer.h

```
bool load(const std::string& filePath);
void play(); void pause(); void stop(); void reset();
void setVolume(float v);       // 0.0~1.0
void setOffset(float ms);      // ms offset
void setLoop(bool l);
float getCurrentTime() const;  // seconds (core)
float getTotalTime() const;
bool  isPlaying() const;
bool  isLoaded() const;
```

---

## SettingsData

**路径：** src/SettingsData.h

```
SettingsData(const std::string& filePath = "settings.json");
~SettingsData();
template T get(key, default);
template void set(key, value);

float getMasterVolume();      void setMasterVolume(float v);
float getNoteSpeed();         void setNoteSpeed(float v);
bool  getFullscreen();        void setFullscreen(bool v);
int   getFpsLimit();          void setFpsLimit(int v);
float getOffset();            void setOffset(float ms);
```

---


## ResourceManager

**路径：** src/ResourceManager.h

`
static ResourceManager& instance();
sf::Texture*     loadTexture(const std::string& path);
sf::Font*        loadFont(const std::string& path);
sf::Shader*      loadShader(const std::string& name, const std::string& fragPath);
sf::SoundBuffer* loadSoundBuffer(const std::string& path);
void clear();
`
## SFML 3 API Notes

```
font.openFromFile("path");              // returns bool, NOT loadFromFile
sf::Text text(font, "string", size);    // no default constructor
event.getIf<sf::Event::KeyPressed>()    // SFML 3 event pattern
key->scancode == sf::Keyboard::Scancode::Escape
music.getStatus() == sf::SoundSource::Status::Playing  // enum class, scope required
```

---

## C++ Constraints

```
// unique_ptr in containers — needs complete type
vector<unique_ptr<IScene>>          // needs #include "IScene.h"

// By-value member — needs complete type
SceneManager m_sceneManager;        // needs #include "SceneManager.h"

// Member order — depends before dependent
sf::Font m_font;                    // declared first
std::optional<sf::Text> m_text;     // declared second (references m_font)
```

## GameplayScene

**路径：** src/GameplayScene.h

`
GameplayScene();
void onEnter() override;
void onExit() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;
ResultData getResultData() const;

// private
void loadChart(const std::string& filePath);
void startGame();
void applySettings();
void buildTracks();
void buildJudgmentLine();
void buildBackground();
void spawnNotes(float currentTime);
void checkJudgment(int track);
void onNoteJudged(JudgeResult result);
void checkHoldRelease(int track);
void autoMissCheck();
bool allNotesProcessed() const;
void endGame();
float getTrackCenterX(int track) const;
`

**文件拆分：** GameplayScene_Setup.cpp (setup) · GameplayScene_Update.cpp (核心循环) · GameplayScene_Render.cpp (渲染) · GameplayScene_Judgment.cpp (判定 + Hold) · GameplayScene_EndGame.cpp (结算)

## ResultData

**路径：** src/GameplayScene.h (在 GameplayScene 类上方定义)

`
struct ResultData {
    int score, maxCombo;
    int perfectCount, greatCount, goodCount, missCount;
    string songTitle;
};
`

---
## MenuScene

**路径：** src/MenuScene.h

**继承：** IScene

```
MenuScene();
void onEnter() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;

// private
void activateItem(int index);
void updateSelectionVisuals();
```

**交互：** ↑↓ 键盘 / 鼠标悬停 + 左键点击。三个选项：Start Game → replace(PackScene)，Settings → (待实现)，Exit → popScene()

---

## PackScene

**路径：** src/PackScene.h

**继承：** IScene

```
PackScene();
void onEnter() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;

// private
void activateItem(int index);
void updateSelectionVisuals();
```

**交互：** ↑↓ 键盘 / 鼠标悬停 + 左键点击。Enter 或点击选中歌曲 → push(GameplayScene)，ESC → popScene() 返回 MenuScene

---

## GameplayScene (Phase 4 新增)

**静态成员：**

| 成员 | 类型 | 作用 |
|------|------|------|
| s_chartPath | static std::string | PackScene 在 push GameplayScene 前设置，onEnter 时据此加载谱面文件；为空则 generateExampleBeatmap |

---
## SettingsScene

**路径：** src/SettingsScene.h

**继承：** IScene

```
SettingsScene();
void onEnter() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;

// private
void refreshDisplay();
void activateItem(int index);
void updateSelectionVisuals();
```

**交互：** ↑↓ + 鼠标悬停/点击 选中设置项，←→ 调整数值，ESC 返回（自动保存到 SettingsData）。

**设置项：**

| 项目 | 范围 | 步长 |
|------|------|------|
| Volume | 0.0 ~ 1.0 | 0.05 |
| Note Speed | 1.0 ~ 10.0 | 0.5 |
| Fullscreen | toggle | — |
| FPS Limit | 30 ~ 240 | 10 |
| Offset (ms) | -200 ~ 200 | 10 |

---

## ResultScene

**路径：** src/ResultScene.h

**继承：** IScene

```
ResultScene(const ResultData& data, const std::string& grade);
void onEnter() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;
```

**交互：** 任意键盘按键或鼠标左键 → requestPop() 返回选歌界面。

**数据传入：** 由 GameplayScene::endGame() 在游戏结束时创建 ResultScene，传入 ResultData + 评级字符串。

---

## PauseScene

**路径：** src/PauseScene.h

**继承：** IScene

```
PauseScene();
void onEnter() override;
void handleEvent(const sf::Event& event) override;
void update(float dt) override;
void render(sf::RenderTarget& target) override;

// private
void activateItem(int index);
void updateSelectionVisuals();
```

**交互：** ↑↓ + 鼠标悬停/点击 选项，ESC = Continue。两个选项：
  - Continue → requestPop() 回到 GameplayScene
  - Return to Menu → 设置 GameplayScene::s_returnToMenu = true，pop 后 GameplayScene 自动退出

---

## GameplayScene (Phase 4 更新)

**新增静态成员：**

| 成员 | 类型 | 作用 |
|------|------|------|
| s_returnToMenu | static bool | PauseScene 设置 → GameplayScene::update() 检测并自动 pop |

**更新行为：**
- handleEvent() 新增 ESC 键拦截 → 暂停音乐 + push(PauseScene)
- update() 新增 s_returnToMenu 检测 → 停止音乐 + requestPop()
- endGame() → requestReplace(ResultScene) 而非直接返回

---

