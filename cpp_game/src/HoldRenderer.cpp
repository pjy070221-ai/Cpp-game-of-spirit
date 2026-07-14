#include "HoldRenderer.h"
#include <algorithm>
#include <cmath>

HoldRenderer::HoldRenderer(
    const std::vector<HoldNoteData>& chart,
    const sf::Font& font,
    float judgmentY, float scrollSpd,
    float trkW, float trkSpc,
    float scrnW, float scrnH)
    : m_font(font)
    , m_judgmentY(judgmentY)
    , m_scrollSpd(scrollSpd)
    , m_screenH(scrnH)
    , m_laneW(trkW)
    , m_laneColors{
        sf::Color(255,80,80),
        sf::Color(80,80,255),
        sf::Color(255,220,80),
        sf::Color(255,80,255)}
{
    float startX = (scrnW - (4 * trkW + 3 * trkSpc)) * 0.5f;
    for (int i = 0; i < 4; ++i)
        m_laneX[i] = startX + i * (trkW + trkSpc);

    m_notes.reserve(chart.size());
    for (auto& d : chart) {
        HoldNoteVisual v;
        v.lane = d.lane; v.startTime = d.startTime; v.endTime = d.endTime;
        initVisual(v);
        m_notes.push_back(v);
    }
}

void HoldRenderer::initVisual(HoldNoteVisual& v) {
    v.bodyShape.setSize({m_laneW, 0});
    v.bodyShape.setFillColor(sf::Color(255,255,255,60));
    v.bodyShape.setOutlineThickness(1.0f);
    v.bodyShape.setOutlineColor(sf::Color(200,200,200,80));
    auto& bc = m_laneColors[v.lane];
    v.headShape.setRadius(15.0f); v.headShape.setOrigin({15,15});
    v.headShape.setFillColor(bc); v.headShape.setOutlineThickness(2);
    v.headShape.setOutlineColor(sf::Color::White);
    v.tailShape.setRadius(10.0f); v.tailShape.setOrigin({10,10});
    v.tailShape.setFillColor(sf::Color(bc.r,bc.g,bc.b,160));
    v.tailShape.setOutlineThickness(1);
    v.tailShape.setOutlineColor(sf::Color(255,255,255,60));
    v.glowRing.setRadius(25); v.glowRing.setOrigin({25,25});
    v.glowRing.setFillColor(sf::Color(255,255,255,0));
    v.glowRing.setOutlineThickness(3);
    v.glowRing.setOutlineColor(sf::Color(255,255,255,0));
}

void HoldRenderer::update(float currentMusicTime) {
    float dt = currentMusicTime - m_lastMusicTime;
    m_lastMusicTime = currentMusicTime;
    //
    while (m_nextIdx < m_notes.size() &&
           currentMusicTime >= m_notes[m_nextIdx].startTime - LEAD)
        m_notes[m_nextIdx++].isAlive = true;

    for (auto& v : m_notes) {
        if (!v.isAlive) continue;
        v.headY = m_judgmentY - (v.startTime - currentMusicTime) * m_scrollSpd;
        v.tailY = m_judgmentY - (v.endTime   - currentMusicTime) * m_scrollSpd;
        float cx = laneCenter(v.lane);
        float bodyH = v.headY - v.tailY;
        if (bodyH < 0) bodyH = 0;
        v.bodyShape.setPosition({m_laneX[v.lane], v.tailY});
        v.bodyShape.setSize({m_laneW, bodyH});
        v.headShape.setPosition({cx, v.headY});
        v.tailShape.setPosition({cx, v.tailY});
        v.glowRing.setPosition({cx, v.headY});

        int phase = m_getPhase ? m_getPhase(v.lane, v.startTime) : 0;
        auto prev = v.visualState;
        if (phase == 0)
            v.visualState = (v.headY < m_judgmentY + 50)
                ? HoldNoteVisual::VisualState::HeadApproaching
                : HoldNoteVisual::VisualState::Idle;
        else if (phase == 1)
            v.visualState = HoldNoteVisual::VisualState::Pressed;
        else
            v.visualState = HoldNoteVisual::VisualState::Released;

        if (v.visualState == HoldNoteVisual::VisualState::Pressed &&
            prev != HoldNoteVisual::VisualState::Pressed && m_getJudgment) {
            auto s = m_getJudgment(v.lane, v.startTime);
            if (!s.empty()) {
                v.judgmentText.emplace(m_font, s, 18);
                v.judgmentText->setFillColor(sf::Color::White);
                auto b = v.judgmentText->getLocalBounds();
                v.judgmentText->setOrigin({b.size.x*0.5f, b.size.y*0.5f});
                v.judgmentText->setPosition({cx, v.headY - 30});
                v.judgmentDisplayTimer = 0.5f;
            }
        }

        applyState(v);

        if (v.tailY > m_screenH + 100) {
            v.isAlive = false; v.judgmentText.reset();
        }
        if (v.judgmentDisplayTimer > 0) {
            v.judgmentDisplayTimer -= dt;
            v.judgmentText->move({0, -dt * 60});
            auto c = v.judgmentText->getFillColor();
            c.a = (uint8_t)((v.judgmentDisplayTimer/0.5f)*255);
            v.judgmentText->setFillColor(c);
            if (v.judgmentDisplayTimer <= 0) v.judgmentText.reset();
        }
    }
}

void HoldRenderer::applyState(HoldNoteVisual& v) {
    auto& bc = m_laneColors[v.lane];
    switch (v.visualState) {
    case HoldNoteVisual::VisualState::Idle:
    case HoldNoteVisual::VisualState::HeadApproaching:
        v.bodyShape.setFillColor(sf::Color(255,255,255,60));
        v.bodyShape.setOutlineColor(sf::Color(200,200,200,80));
        v.headShape.setRadius(15); v.headShape.setOrigin({15,15});
        v.headShape.setFillColor(bc); v.headShape.setOutlineColor(sf::Color::White);
        v.headShape.setOutlineThickness(2);
        v.tailShape.setFillColor(sf::Color(bc.r,bc.g,bc.b,160));
        v.tailShape.setOutlineColor(sf::Color(255,255,255,60));
        v.glowRing.setFillColor(sf::Color(255,255,255,0));
        v.glowRing.setOutlineColor(sf::Color(255,255,255,0));
        break;
    case HoldNoteVisual::VisualState::Pressed:
        v.bodyShape.setFillColor(sf::Color(255,255,255,180));
        v.bodyShape.setOutlineColor(sf::Color(255,255,255,200));
        v.headShape.setRadius(20); v.headShape.setOrigin({20,20});
        v.headShape.setFillColor(sf::Color(
            (uint8_t)std::min(255, bc.r+80),
            (uint8_t)std::min(255, bc.g+80),
            (uint8_t)std::min(255, bc.b+80)));
        v.headShape.setOutlineColor(sf::Color::White);
        v.headShape.setOutlineThickness(3);
        v.glowRing.setFillColor(sf::Color(255,255,255,40));
        v.glowRing.setOutlineColor(sf::Color(255,255,255,80));
        v.tailShape.setFillColor(sf::Color(bc.r,bc.g,bc.b,160));
        v.tailShape.setOutlineColor(sf::Color(255,255,255,120));
        break;
    case HoldNoteVisual::VisualState::Released:
        v.bodyShape.setFillColor(sf::Color(255,255,255,30));
        v.bodyShape.setOutlineColor(sf::Color(200,200,200,30));
        v.headShape.setRadius(12); v.headShape.setOrigin({12,12});
        v.headShape.setFillColor(sf::Color(128,128,128));
        v.headShape.setOutlineColor(sf::Color(80,80,80));
        v.headShape.setOutlineThickness(1);
        v.tailShape.setFillColor(sf::Color(bc.r,bc.g,bc.b,60));
        v.tailShape.setOutlineColor(sf::Color(255,255,255,20));
        v.glowRing.setFillColor(sf::Color(255,255,255,0));
        v.glowRing.setOutlineColor(sf::Color(255,255,255,0));
        break;
    }
}

void HoldRenderer::draw(sf::RenderTarget& target) const {
    for (auto& v : m_notes) {
        if (!v.isAlive) continue;
        target.draw(v.bodyShape);
        target.draw(v.tailShape);
        if (v.glowRing.getFillColor().a > 0) target.draw(v.glowRing);
        target.draw(v.headShape);
        if (v.judgmentText.has_value()) target.draw(*v.judgmentText);
    }
}
