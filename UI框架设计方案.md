# 第二位队友（Theresa0624）代码分析 — 继承体系与架构总结

> 分析来源：https://github.com/pjy070221-ai/Cpp-game-of-spirit  
> 提交者：Theresa0624（提交 e275699 "添加了项目文件"）  
> 路径：`Demo/FI/FI/`  
> 开发环境：Visual Studio 2022 + SFML 3.1  
> 项目名：Cross Beat / Abyssal Beat  
> 与第一位队友对比：采用了**真正的继承树架构**，具备完整的场景管理系统

---

## 一、整体架构图

```
main.cpp
  └── Application（顶层容器）
        └── SceneManager（场景栈管理器）
              ├── IScene（抽象基类）
              │     ├── TitleScene      — 标题/入场动画
              │     ├── MenuScene       — 主菜单
              │     ├── PackScene       — 曲包选择
              │     ├── GameplayScene   — "Cross Beat"十字游玩
              │     ├── ResultScene     — 结算界面
              │     └── SettingsScene   — 设置界面
              │
              ├── 基础支撑
              │     ├── ResourceManager — 资源管理（单例）
              │     ├── ParticleSystem  — 粒子系统
              │     ├── AnomalySystem   — 异象演出时间轴
              │     └── Easing.h        — 缓动函数库
              │
              └── 游玩核心
                    ├── Note（struct）      — 音符数据
                    ├── ChartLoader         — 谱面加载器
                    └── AutoPlayer         — 自动演示
```

关键区别：这是一个**以场景管理为核心**的游戏框架，和第一位队友的"Game 大类一把梭"完全不同。

---

## 二、IScene —— 场景抽象基类（继承树根）

```cpp
// scenes/IScene.h
class IScene {
public:
    virtual ~IScene() = default;

    // 生命周期钩子
    virtual void onEnter() {}
    virtual void onExit() {}

    // 事件/更新/渲染
    virtual void handleEvent(const sf::Event& event) {}
    virtual void update(float dt) = 0;        // 纯虚
    virtual void render(sf::RenderTarget& target) = 0;  // 纯虚

    // 延迟场景切换请求（安全设计，避免 update 中 delete this）
    void requestPush(std::unique_ptr<IScene> scene);
    void requestPop();
    void requestReplace(std::unique_ptr<IScene> scene);

    // SceneManager 在入栈时注入指针
    void setSceneManager(SceneManager* manager);

protected:
    SceneManager* m_sceneManager = nullptr;
};
```

**设计要点：**
- `onEnter()` / `onExit()` 场景生命周期钩子，用于初始化 / 清理
- `update()` 和 `render()` 是纯虚函数，强迫每个子类实现
- `requestPush/Pop/Replace` 不会立即执行，而是挂起到帧末，**防止了在 update() 中 delete this 导致的崩溃**
- 每个场景持有 `SceneManager*` 指针，由 SceneManager 在入栈时注入

---

## 三、6 个派生场景类

### 3.1 TitleScene —— 最完整的场景（752 行实现）

```cpp
// scenes/TitleScene.h
class TitleScene : public IScene {
    // 入场动画状态机
    enum class State { Loading, Shattering, TitleReveal, Idle };
    State m_state;

    // Loading 阶段 (2.5s) — 纯黑背景 + 进度条 + 星空粒子
    float m_loadProgress;
    sf::RectangleShape m_loadTrack, m_loadFill;
    sf::Text m_loadLabel;           // "LOADING..."

    // Shattering 阶段 (0.8s) — 裂纹从中心蔓延 + 白闪 + 碎屑背景淡入
    float m_shatterTimer;
    struct ShatterCrack { float angle, maxLen, delay; };
    std::vector<ShatterCrack> m_shatterCrackData;
    sf::VertexArray m_shatterCrackGeom;   // 用 Lines 绘制

    // TitleReveal 阶段 — 标题飞入 + 副标题淡入
    // CharData: 每个字符独立浮动（正/余弦波，各不同相位）
    struct CharData {
        sf::Text text;
        float baseSize, sizeVariance, animPhase;
        float posX, posY;
    };
    std::vector<CharData> m_titleChars;  // "Abyssal Beat" 12 个字符
    std::vector<CharData> m_subChars;    // "Demo" 4 个字符

    // Idle 阶段 — 常驻效果（故障闪烁、呼吸、粒子）
    float m_glitchTimer;     // 距下一次触发倒计时
    float m_glitchDuration;  // 当前抽抽剩余时间
    float m_glitchIntensity; // 当前抽抽强度 [0~1]
    void triggerGlitch();    // 随机触发一次 Glitch

    // 分层渲染（共 9 层）：碎屑背景 → 暗角渐变 → 能量脉动 → 裂纹呼吸
    // → 浮尘粒子 → 锐利星空 → 标题文字(RGB分裂) → 故障闪白 → "Press ENTER"
    sf::VertexArray m_bgShards;    // 碎屑背景三角网（一次性构建）
    sf::VertexArray m_bgGradient;  // 暗角渐变
    sf::VertexArray m_bgEnergy;    // 中心能量 TriangleFan
    sf::VertexArray m_bgCracks;    // 裂纹呼吸
    sf::VertexArray m_bgFlash;     // 故障闪白
    ParticleSystem m_stars;  // 星空
    ParticleSystem m_dust;   // 浮尘
    sf::RenderTexture m_textLayer;  // 文字预渲染纹理
};
```

最复杂的场景，包含 4 个动画状态和 9 层渲染。使用 `VertexArray` 一次性构建 + 运行时只改顶点颜色的策略优化性能。

### 3.2 GameplayScene —— "Cross Beat" 十字交叉式游玩

```cpp
// scenes/GameplayScene.h
class GameplayScene : public IScene {
    // 谱面数据
    Chart m_chart;
    float m_songTime;            // 歌曲时间 (ms)
    size_t m_nextIndex;

    // 十字旋转系统
    float m_crossAngle;          // 十字当前旋转角度 (rad)
    static constexpr float CROSS_SPEED = 0.35f;  // 旋转速度 (rad/s)
    static constexpr float CX = 960.f, CY = 500.f; // 十字中心

    sf::Vector2f armDir(int arm) const {
        float a = m_crossAngle + arm * 1.5707963f; // arm * PI/2
        return { std::cos(a), std::sin(a) };
    }

    // 音符运行时状态
    struct NoteRuntime {
        int   arm = 0;           // 所属臂 (0-3)
        float distance = 650.f;  // 当前距中心距离
        bool  hit = false;
        bool  active = false;
        bool  holding = false;   // Hold 持续中
    };
    std::vector<NoteRuntime> m_noteRuntimes;

    // Flick 独立系统（在臂间生成弧线箭头）
    struct FlickVisual { int fromArm, toArm; bool cw; float life; };
    std::vector<FlickVisual> m_flicks;

    // 分数 & 连击
    int m_score = 0;
    int m_combo = 0;
    int m_maxCombo = 0;

    // 判定弹出文字
    struct JudgePop { sf::Text text; float life; };
    std::vector<JudgePop> m_judgements;

    // 粒子
    ParticleSystem m_hitFX;   // 打击特效
    ParticleSystem m_stars;   // 背景星空

    // 视觉
    sf::VertexArray m_bgGradient;
    sf::RectangleShape m_noteShape;

    // UI
    sf::Text m_titleText, m_scoreText, m_comboText, m_hintText;
};
```

**核心创意**：音符从四个方向沿十字臂飞向中心判定点，十字本身缓慢旋转。背后的数学只有 `sin` / `cos`，但视觉效果独特。判定区域是距离中心 `HIT_DIST = 15px` 的圆形区域。

### 3.3 MenuScene —— 主菜单

（未完整读取 .h 内容，从目录结构推测包含选项列表、背景动效等）

### 3.4 PackScene —— 曲包选择

```cpp
// scenes/PackScene.h
class PackScene : public IScene {
    // 曲目列表、封面预览、难度展示
};
```

### 3.5 ResultScene —— 结算界面

```cpp
// scenes/ResultScene.h
class ResultScene : public IScene {
    // 评级动画、分数统计、Perfect/Great/Miss 分布
};
```

### 3.6 SettingsScene —— 设置界面

```cpp
// scenes/SettingsScene.h
class SettingsScene : public IScene {
    // 音量、下落速度、按键映射等
};
```

---

## 四、SceneManager —— 场景栈管理器

```cpp
class SceneManager {
public:
    void pushScene(std::unique_ptr<IScene> scene);
    void popScene();
    void replaceScene(std::unique_ptr<IScene> scene);

    void handleEvent(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderTarget& target);
    bool isEmpty() const;

private:
    void applyPendingChanges();   // 在帧末执行实际切换

    struct PendingChange {
        Action action = Action::None;
        std::unique_ptr<IScene> scene;
    };

    std::vector<std::unique_ptr<IScene>> m_scenes;  // 场景栈
    PendingChange m_pending;   // 挂起的切换请求
};
```

**核心安全设计：** 场景切换请求不立即执行。Scene A 的 `update()` 中调用 `requestReplace(B)`，实际切换发生在 `update()` 返回之后。这避免了"在 A 的成员函数中 delete A"的经典 C++ 灾难。

**渲染策略：** 从栈底到栈顶依次 `render()`，栈底作为背景可见（如果栈顶场景有透明区域）。

---

## 五、Application —— 顶层容器

```cpp
class Application {
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    void run();   // 主循环

private:
    void processEvents();  // 轮询窗口事件并分发给当前场景
    void update();         // 更新场景 + 应用延迟切换
    void render();         // 离屏渲染 → bloom → 窗口显示

    sf::RenderWindow m_window;
    SceneManager     m_sceneManager;
    sf::Clock        m_clock;
    float            m_deltaTime = 0.0f;
};
```

主循环流程：`processEvents()` → `update()` → `render()`，标准的三部曲。内置信道渲染管线。

---

## 六、基础支撑类（4 个）

### 6.1 ResourceManager（单例模式）

```cpp
class ResourceManager {
public:
    static ResourceManager& instance();  // 单例

    sf::Texture*     loadTexture(const std::string& path);
    sf::Font*        loadFont(const std::string& path);
    sf::Shader*      loadShader(const std::string& name, const std::string& fragPath, const std::string& vertPath = "");
    sf::SoundBuffer* loadSoundBuffer(const std::string& path);
    void clear();

private:
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>>     m_textures;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>>        m_fonts;
    std::unordered_map<std::string, std::unique_ptr<sf::Shader>>      m_shaders;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_sounds;
};
```

- 文件只从磁盘加载一次，后续返回缓存指针
- 返回裸指针，所有权在内部（`unique_ptr`）
- 加载失败返回 `nullptr`

### 6.2 ParticleSystem（高性能粒子）

```cpp
class ParticleSystem {
public:
    struct Particle {
        sf::Vector2f position, velocity;
        sf::Color color;
        float life, maxLife, size;
    };

    // 爆发式发射 — 从一点向四周喷射
    void emit(const sf::Vector2f& pos, int count, const sf::Color& color,
              float speedMin, float speedMax, float lifeMin, float lifeMax,
              float sizeMin, float sizeMax);

    // 持续性补充 — 在区域内保持 targetCount 个粒子
    void spawnStars(float dt, const sf::FloatRect& area, int targetCount = 200);

    void update(float dt);
    void render(sf::RenderTarget& target) const;  // 使用 VertexArray
    void clear();

private:
    std::vector<Particle> m_particles;
};
```

- **VertexArray 批量渲染**：每个粒子 6 个顶点（2 个三角形 = 1 个 Quad），单次 draw call 绘制全部
- **加法混合**：`sf::BlendMode::Add` 产生发光效果
- 200 粒子 × 6 顶点 = 1200 顶点/帧，对现代 GPU 可忽略

### 6.3 AnomalySystem（异象演出时间轴）

```cpp
enum class AnomalyType {
    ScreenShake, NoteSpeedChange, LaneShift, ColorInvert,
    ChromaticRift, Flash, NoteFreeze, Reverse,
    JudgementLineSplit, PerspectiveShift
};

struct AnomalyEvent {
    float triggerTime;      // 触发时间 (ms)
    AnomalyType type;       // 类型
    float duration;         // 持续时间 (ms)
    std::unordered_map<std::string, float> params;  // 可扩展参数
    bool triggered, finished;  // 运行时状态
};

class AnomalySystem {
public:
    void loadFromNotes(const std::vector<Note>& notes);  // 从谱面 Note 提取
    void setEvents(const std::vector<AnomalyEvent>& events);
    void update(float songPositionMs, float dt);
    bool isActive(AnomalyType type) const;
    float getIntensity(AnomalyType type) const;  // 多事件叠加取最大
    float getParam(const std::string& key, float defaultValue) const;
    void reset();
};
```

- 时间轴驱动：所有事件按 `triggerTime` 排序，`update()` 中检查触发
- 活跃列表记录正在播放的效果，超时自动移除
- 可通过 `NoteType::Anomaly` 类型的 Note 从谱面中声明

### 6.4 Easing.h（缓动函数库）

```
// 包含缓动函数库，提供 easeOutExpo、easeOutBack、easeOutCubic 等
// 用于 TitleScene 的入场动画（标题飞入、副标题淡入、提示文字闪入）
```

---

## 七、谱面格式（JSON）

```json
{
  "meta": {
    "title": "The Last Arc -Final Boss-",
    "artist": "Artist Name",
    "bpm": 200.0,
    "offset": -30,
    "difficulty": 11,
    "audioFile": "boss_song.ogg"
  },
  "notes": [
    { "type": "tap",   "lane": 1, "time": 1000.0 },
    { "type": "hold",  "lane": 4, "time": 2500.0, "endTime": 3500.0 },
    { "type": "anomaly", "time": 10500.0, "effect": "flash",
      "duration": 500.0, "params": { "intensity": 0.8 } }
  ]
}
```

时间单位为毫秒。支持 tap、hold、anomaly 三种音符类型。异常事件不参与游玩判定，只触发视觉特效。

---

## 八、两位队友代码对比

| 方面 | 第一位队友（pjy070221-ai） | 第二位队友（Theresa0624） |
|------|---------------------------|--------------------------|
| **架构风格** | 平坦架构：Game 大类 909 行 | 继承树：IScene + 6 个派生场景 |
| **场景管理** | 无，菜单用 Game 内 switch 硬编码 | SceneManager 栈管理 + 延迟切换 |
| **游玩方式** | 传统 4K 下落式（音符下落到判定线） | "Cross Beat" 十字交叉式（音符飞向中心） |
| **纹理/字体** | 直接系统字体路径 | ResourceManager 单例缓存 |
| **粒子系统** | CircleShape 数组直接在 Game 中管理 | VertexArray 批量渲染的 ParticleSystem |
| **特效果系统** | 无 | AnomalySystem + Easing 缓动 |
| **设置系统** | SettingsData（variant+模板，很完善） | 无独立设置类，SettingsScene 待实现 |
| **音频管理** | MusicPlayer 封装 sf::Music | 未实现（谱面 meta 中指定 audioFile） |
| **代码量** | ~700+ 行（Game.cpp 占 909 行） | ~5000+ 行（39 个文件，TitleScene 占 752 行） |
| **完成度** | 可编译运行，判定和计分已实现 | 框架完整，但核心 gameplay 循环待完善 |

---

## 九、开发思路总结

这位队友的代码体现的是**工程化、结构化的开发思路**：

1. **场景驱动架构** — 每个游戏画面是一个独立的 `IScene` 子类，各管各的生命周期。SceneManager 用栈管理切换，安全又灵活。

2. **注重视觉品质** — TitleScene 有完整的入场动画序列（加载→碎裂→标题飞入→待机），9 层渲染，glitch 抽抽效果，字符级动画。这不是"凑合能看"的水平，而是有审美追求的封装。

3. **性能意识** — VertexArray 零分配渲染粒子、一次性构建 + 运行时只改顶点颜色的背景网格、离屏预渲染文字纹理——这些优化在音游中很有价值。

4. **可扩展性预留** — AnomalySystem 预留了 10 种异象类型、谱面 JSON 支持自定义 params、NoteType 预设了 Flick（滑动）类型但标了"暂未实现渲染"。

5. **风险点** — GameplayScene 的十字玩法（625 行实现）已完成度待确认；缺少音频同步；缺少设置持久化（SettingsScene 框架在但依赖的序列化未实现）。
