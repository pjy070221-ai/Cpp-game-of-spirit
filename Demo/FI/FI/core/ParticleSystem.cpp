#include "ParticleSystem.h"
#include <random>
#include <cmath>

// 静态随机数生成器
static std::mt19937& rng()
{
    static std::mt19937 gen(42); // 固定种子保证可复现
    return gen;
}

void ParticleSystem::emit(
    const sf::Vector2f& pos,
    int count,
    const sf::Color& color,
    float speedMin, float speedMax,
    float lifeMin, float lifeMax,
    float sizeMin, float sizeMax)
{
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265359f);
    std::uniform_real_distribution<float> speedDist(speedMin, speedMax);
    std::uniform_real_distribution<float> lifeDist(lifeMin, lifeMax);
    std::uniform_real_distribution<float> sizeDist(sizeMin, sizeMax);

    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.position = pos;
        float angle = angleDist(rng());
        float speed = speedDist(rng());
        p.velocity = sf::Vector2f(
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
        p.color = color;
        p.maxLife = lifeDist(rng());
        p.life = p.maxLife;
        p.size = sizeDist(rng());
        m_particles.push_back(p);
    }
}

void ParticleSystem::spawnStars(float dt, const sf::FloatRect& area, int targetCount)
{
    // 如果粒子不够，补充新星空粒子
    int deficit = targetCount - static_cast<int>(m_particles.size());
    if (deficit <= 0) return;

    std::uniform_real_distribution<float> xDist(area.position.x, area.position.x + area.size.x);
    std::uniform_real_distribution<float> yDist(area.position.y, area.position.y + area.size.y);
    std::uniform_real_distribution<float> lifeDist(1.0f, 5.0f);
    std::uniform_real_distribution<float> alphaDist(80.0f, 220.0f);

    for (int i = 0; i < std::min(deficit, 5); ++i)  // 每帧最多补 5 个
    {
        Particle star;
        star.position = { xDist(rng()), yDist(rng()) };
        star.velocity = { 0.0f, 20.0f + yDist(rng()) * 0.1f };  // 缓慢下移
        star.color = sf::Color(200, 220, 255, static_cast<uint8_t>(alphaDist(rng())));
        star.maxLife = lifeDist(rng());
        star.life = star.maxLife;
        star.size = 1.0f + yDist(rng()) * 0.002f; // 1~3 像素
        m_particles.push_back(star);
    }
}

void ParticleSystem::update(float dt)
{
    for (auto& p : m_particles)
    {
        p.life -= dt;
        p.position += p.velocity * dt;
    }

    // 移除死亡粒子（swap-and-pop 高效删除）
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }
        ),
        m_particles.end()
    );
}

void ParticleSystem::render(sf::RenderTarget& target) const
{
    if (m_particles.empty()) return;

    // Additive blend for glow effect (SFML 3.1 API)
    sf::BlendMode additive;
    additive.colorSrcFactor = sf::BlendMode::Factor::SrcAlpha;
    additive.colorDstFactor = sf::BlendMode::Factor::One;
    additive.colorEquation  = sf::BlendMode::Equation::Add;
    additive.alphaSrcFactor = sf::BlendMode::Factor::One;
    additive.alphaDstFactor = sf::BlendMode::Factor::One;
    additive.alphaEquation  = sf::BlendMode::Equation::Add;

    // 构建顶点数组 — 每个粒子一个 Quad
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);
    vertices.resize(m_particles.size() * 6); // 每个 Quad = 6 个顶点

    for (size_t i = 0; i < m_particles.size(); ++i)
    {
        const auto& p = m_particles[i];
        float alpha = p.life / p.maxLife;  // 衰减系数
        sf::Color c = p.color;
        c.a = static_cast<uint8_t>(static_cast<float>(c.a) * alpha);

        float halfSize = p.size * 0.5f;
        sf::Vector2f topLeft     = p.position + sf::Vector2f(-halfSize, -halfSize);
        sf::Vector2f topRight    = p.position + sf::Vector2f( halfSize, -halfSize);
        sf::Vector2f bottomLeft  = p.position + sf::Vector2f(-halfSize,  halfSize);
        sf::Vector2f bottomRight = p.position + sf::Vector2f( halfSize,  halfSize);

        size_t idx = i * 6;
        // 三角形 1: topLeft → topRight → bottomRight
        vertices[idx + 0].position = topLeft;     vertices[idx + 0].color = c;
        vertices[idx + 1].position = topRight;    vertices[idx + 1].color = c;
        vertices[idx + 2].position = bottomRight; vertices[idx + 2].color = c;
        // 三角形 2: topLeft → bottomRight → bottomLeft
        vertices[idx + 3].position = topLeft;     vertices[idx + 3].color = c;
        vertices[idx + 4].position = bottomRight; vertices[idx + 4].color = c;
        vertices[idx + 5].position = bottomLeft;  vertices[idx + 5].color = c;
    }

    // 用加法混合绘制
    sf::RenderStates states;
    states.blendMode = additive;
    target.draw(vertices, states);
}
