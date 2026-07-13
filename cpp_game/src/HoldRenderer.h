#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include <functional>

struct HoldNoteData {
    int   lane = 0;
    float startTime = 0.0f;
    float endTime = 0.0f;
};

struct HoldNoteVisual {
    int     lane = 0;
    float   startTime = 0.0f, endTime = 0.0f;
    float   headY = 0.0f, tailY = 0.0f;
    bool    isAlive = false;
    enum class VisualState { Idle, HeadApproaching, Pressed, Released };
    VisualState visualState = VisualState::Idle;
    float pressOffset = 0.0f, releaseOffset = 0.0f;
    sf::CircleShape     glowRing;
    sf::CircleShape     headShape;
    sf::CircleShape     tailShape;
    sf::RectangleShape  bodyShape;
    std::optional<sf::Text> judgmentText;
    float judgmentDisplayTimer = 0.0f;
};

class HoldRenderer {
public:
    using GetPhaseFn    = std::function<int(int, float)>;
    using GetJudgmentFn = std::function<std::string(int, float)>;

    HoldRenderer(
        const std::vector<HoldNoteData>& chart,
        const sf::Font& font,
        float judgmentY  = 550.0f,
        float scrollSpd  = 400.0f,
        float trkW       = 80.0f,
        float trkSpc     = 20.0f,
        float scrnW      = 1280.0f,
        float scrnH      = 720.0f);

    void update(float currentMusicTime);
    void draw(sf::RenderTarget& target) const;
    void setScrollSpeed(float s) { m_scrollSpd = s; }
    void setPhaseCallback(GetPhaseFn fn)    { m_getPhase = fn; }
    void setJudgmentCallback(GetJudgmentFn fn) { m_getJudgment = fn; }

private:
    const sf::Font& m_font;
    float m_judgmentY, m_scrollSpd, m_screenH, m_laneW;
    float m_laneX[4];
    sf::Color m_laneColors[4];
    std::vector<HoldNoteVisual> m_notes;
    size_t m_nextIdx = 0;
    float  m_lastMusicTime = 0.0f;
    GetPhaseFn    m_getPhase;
    GetJudgmentFn m_getJudgment;

    void initVisual(HoldNoteVisual& v);
    void applyState(HoldNoteVisual& v);
    float laneCenter(int l) const { return m_laneX[l] + m_laneW * 0.5f; }
    static constexpr float LEAD = 2.0f;
};
