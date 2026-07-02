#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

// ╔══════════════════════════════════════════════════════════╗
// ║  ParticleSystem — 高性能粒子系统                           ║
// ╠══════════════════════════════════════════════════════════╣
// ║                                                          ║
// ║  【功能】管理大量 2D 粒子（星空、打击特效、尘埃等）。       ║
// ║                                                          ║
// ║  【设计特点】                                              ║
// ║    1. 纯 CPU 粒子 — 用 std::vector<Particle> 存储，        ║
// ║       每帧 update() 更新位置/生命。                       ║
// ║    2. VertexArray 渲染 — render() 时构建 sf::VertexArray   ║
// ║       （Triangles），每个粒子 6 个顶点（2 三角 = 1 Quad）。 ║
// ║       单次 draw call 绘制全部粒子。                        ║
// ║    3. 加法混合 — sf::BlendMode 的 Add 模式产生发光效果。    ║
// ║    4. Swap-and-pop 清除 — 死亡粒子用 remove_if + erase，   ║
// ║       高效移除。                                          ║
// ║                                                          ║
// ║  【性能考量】                                              ║
// ║    200 粒子 × 6 顶点 = 1200 顶点/帧，对现代 GPU 可忽略。    ║
// ║    VertexArray 每帧重建（分配）是主要开销，                ║
// ║    但 1200 顶点的分配 + 填充在现代 CPU 上 < 0.1ms。        ║
// ║                                                          ║
// ║  【与其他类的关系】                                        ║
// ║    - TitleScene ：持有 m_stars + m_dust 两个实例            ║
// ║    - GameplayScene ：持有 m_hitEffects 打击特效实例         ║
// ║    - 独立于任何场景，可在任意 RenderTarget 上渲染            ║
// ║                                                          ║
// ║  【用法】                                                  ║
// ║    particles.emit(pos, 50, sf::Color::Cyan, 100,300, ...);║
// ║    particles.update(dt);                                  ║
// ║    particles.render(window);                              ║
// ║                                                          ║
// ╚══════════════════════════════════════════════════════════╝
class ParticleSystem
{
public:
    struct Particle
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color    color;
        float        life    = 0.0f;
        float        maxLife = 1.0f;
        float        size    = 2.0f;
    };

    ParticleSystem() = default;

    // 爆发式发射 — 从一点向四周喷射 count 个粒子
    void emit(
        const sf::Vector2f& pos, int count, const sf::Color& color,
        float speedMin = 50.f, float speedMax = 200.f,
        float lifeMin = 0.3f, float lifeMax = 1.0f,
        float sizeMin = 1.f, float sizeMax = 3.f
    );

    // 持续式补充 — 在区域内保持 targetCount 个粒子（用于背景星空）
    void spawnStars(float dt, const sf::FloatRect& area, int targetCount = 200);

    void update(float dt);
    void render(sf::RenderTarget& target) const;
    void clear() { m_particles.clear(); }
    size_t count() const { return m_particles.size(); }

private:
    std::vector<Particle> m_particles;
};
