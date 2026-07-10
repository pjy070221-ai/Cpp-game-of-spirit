# 音游 "节奏大师-like" 初步继承体系设计

> 开发环境：Visual Studio 2022 + SFML 3.1  
> 按键映射：D / F / J / K（四键下落式）  
> 参考原型：腾讯《节奏大师》

---

## 一、继承体系总览

```
GameObject (抽象基类)
├── Note            (单个点击音符)
│   └── HoldNote    (长按音符)
├── Lane            (单条轨道)
├── JudgmentLine    (判定线)
└── HitEffect       (打击特效)
```

---

## 二、基类 GameObject

```cpp
// GameObject.h
#pragma once
#include <SFML/Graphics.hpp>

class GameObject {
public:
    GameObject() = default;
    virtual ~GameObject() = default;

    // 每一帧更新，dt 为帧时间（秒）
    virtual void update(float dt) = 0;

    // 设置/获取基础属性
    void setPosition(const sf::Vector2f& pos);
    void setVelocity(const sf::Vector2f& vel);
    void setSprite(const sf::Sprite& sprite);

    const sf::Vector2f& getPosition() const;
    const sf::Vector2f& getVelocity() const;
    const sf::Sprite&   getSprite() const;

protected:
    sf::Vector2f m_position;   // 位置
    sf::Vector2f m_velocity;   // 速度（像素/秒）
    sf::Sprite   m_sprite;     // 精灵
};
```

---

## 三、派生类

### 3.1 Note —— 单击音符

```cpp
// Note.h
#pragma once
#include "GameObject.h"

enum class NoteType {
    Tap,   // 单点
    Hold   // 长按（由 HoldNote 处理）
};

class Note : public GameObject {
public:
    Note(int laneIndex, float targetTime, float speed);

    void update(float dt) override;

    // 判断音符是否进入判定区域
    bool isInJudgmentZone() const;
    // 标记已击中 / 已错过
    void onHit();
    void onMiss();
    // 是否已被处理
    bool isProcessed() const;

protected:
    int     m_laneIndex;   // 所属轨道索引 0~3 → D,F,J,K
    float   m_targetTime;  // 理论击打时间点（相对曲目时间）
    float   m_speed;       // 下落速度
    bool    m_processed;   // 是否已判定（命中/Miss）
    NoteType m_type;       // 音符类型（Tap / Hold）
};
```

### 3.2 HoldNote —— 长按音符

```cpp
// HoldNote.h
#pragma once
#include "Note.h"

class HoldNote : public Note {
public:
    HoldNote(int laneIndex, float targetTime, float holdDuration, float speed);

    void update(float dt) override;

    // 开始按压、持续按压中、松开
    void onPress();
    void onRelease();

    bool isHolding() const;

private:
    float    m_holdDuration;   // 长按持续时长（秒）
    float    m_holdElapsed;    // 已按压时长
    bool     m_isHolding;      // 是否正在按压中
    sf::Sprite m_bodySprite;   // 长按条的身体（拉伸显示）
};
```

### 3.3 Lane —— 单条轨道

```cpp
// Lane.h
#pragma once
#include "GameObject.h"
#include <vector>
#include <memory>

class Note; // 前向声明

class Lane : public GameObject {
public:
    Lane(int laneIndex, float x, float width, float height);

    void update(float dt) override;

    void addNote(std::unique_ptr<Note> note);

    int getLaneIndex() const;

    // 从轨道中移除已判定的音符
    void removeProcessedNotes();

private:
    int                                    m_laneIndex;    // 0~3
    sf::RectangleShape                     m_bgRect;       // 轨道背景 / 高亮
    float                                  m_width;
    float                                  m_height;
    float                                  m_x;
    std::vector<std::unique_ptr<Note>>     m_notes;        // 该轨道上的音符
};
```

### 3.4 JudgmentLine —— 判定线

```cpp
// JudgmentLine.h
#pragma once
#include "GameObject.h"

class JudgmentLine : public GameObject {
public:
    JudgmentLine(float y, float width);

    void update(float dt) override;

    float getY() const;
    void  setY(float y);

    // 判定区域的上下边界（偏移量）
    float getJudgmentRange() const;

private:
    sf::RectangleShape m_lineShape;  // 判定线视觉
    float              m_width;
    float              m_judgmentRange; // 判定有效偏移（像素）
};
```

### 3.5 HitEffect —— 打击特效

```cpp
// HitEffect.h
#pragma once
#include "GameObject.h"

enum class JudgmentGrade {
    Perfect,
    Great,
    Good,
    Miss
};

class HitEffect : public GameObject {
public:
    HitEffect(const sf::Vector2f& pos, JudgmentGrade grade);

    void update(float dt) override;

    bool isFinished() const;

private:
    JudgmentGrade m_grade;   // 判定等级
    float         m_lifetime;   // 特效总存活时间（秒）
    float         m_elapsed;    // 已存活时间
};
```

---

## 四、GameWorld —— 全局管理器

```cpp
// GameWorld.h
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class GameObject;
class Lane;
class JudgmentLine;

class GameWorld {
public:
    GameWorld() = default;
    ~GameWorld() = default;

    // 核心循环
    void update(float dt);
    void render(sf::RenderWindow& window);

    // 对象管理
    void addObject(std::unique_ptr<GameObject> obj);
    void removeAllObjects();

    // 轨道 & 判定线快捷访问
    void addLane(std::unique_ptr<Lane> lane);
    void setJudgmentLine(std::unique_ptr<JudgmentLine> line);

    Lane* getLane(int index) const;
    JudgmentLine* getJudgmentLine() const;

    // 输入处理
    void onKeyPress(int laneIndex);   // 按键按下
    void onKeyRelease(int laneIndex); // 按键抬起（用于 HoldNote）

    // 计分 & Combo
    int  getScore() const;
    int  getCombo() const;
    void addScore(JudgmentGrade grade);
    void resetCombo();

private:
    std::vector<std::unique_ptr<GameObject>> m_objects;  // 全部游戏对象
    // 同时单独持有 Lane 指针，避免频繁 dynamic_cast
    std::vector<Lane*> m_lanePtrs;
    JudgmentLine*      m_judgmentLinePtr = nullptr;

    int m_score = 0;
    int m_combo = 0;
};
```

---

## 五、类间关系示意

```
GameWorld
 ├── m_objects : vector<unique_ptr<GameObject>>
 │    ├── Lane[0]  ─── Note[], Note[], HoldNote[] ...
 │    ├── Lane[1]  ─── Note[], Note[] ...
 │    ├── Lane[2]  ─── Note[], HoldNote[] ...
 │    ├── Lane[3]  ─── Note[], Note[] ...
 │    ├── JudgmentLine
 │    └── HitEffect[]
 │
 ├── m_lanePtrs (快速索引，避免 dynamic_cast)
 └── m_judgmentLinePtr
```

- `GameWorld` 持有一份 `unique_ptr<GameObject>` 的 vector，拥有全部对象的生命周期。
- `Lane` 内部又持有一份 `unique_ptr<Note>` vector，管理属于该轨道的音符。
- `GameWorld` 的 `update()` 遍历 `m_objects` 调用每个对象的 `update(dt)`；
  各 `Lane` 的 `update()` 再遍历内部 `m_notes`，驱动每个 Note 下落。
- `JudgmentLine` 维护一个 Y 坐标和一个判定范围偏移量，供 `Note` 在 `update` 中判断是否进入判定区。

---

## 六、后续开发方向（供参考）
1. **谱面加载**：解析 `.json` / `.txt` 格式的谱面文件，生成 `Note` 和 `HoldNote` 序列。
2. **输入系统**：读取键盘 D/F/J/K，映射为 `0~3` 的轨道索引，调用 `GameWorld::onKeyPress/release`。
3. **音频同步**：使用 `sf::Music` 或音频库，计算当前播放位置作为 `m_targetTime` 的比对基准。
4. **UI 层**：分数、Combo、曲目进度条、开始/暂停/结算面板（建议与 GameObject 继承体系分离，用独立 UI 模块）。
5. **渲染优化**：背景动效、粒子系统、轨道流光动画。
