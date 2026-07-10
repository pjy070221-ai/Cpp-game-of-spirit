#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

void ParticleSystem::emit(const sf::Vector2f& pos, int count, const sf::Color& c,
                          float spdMin, float spdMax,
                          float lifeMin, float lifeMax,
                          float sizeMin, float sizeMax)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> speedDist(spdMin, spdMax);
    std::uniform_real_distribution<float> lifeDist(lifeMin, lifeMax);
    std::uniform_real_distribution<float> sizeDist(sizeMin, sizeMax);

    for (int i = 0; i < count; ++i) {
        float angle = angleDist(rng);
        float speed = speedDist(rng);

        Particle p;
        p.position = pos;
        p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.color = c;
        p.life = lifeDist(rng);
        p.maxLife = p.life;
        p.size = sizeDist(rng);
        m_particles.push_back(p);
    }
}

void ParticleSystem::spawnStars(float dt, const sf::FloatRect& area, int targetCount) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> xDist(area.position.x, area.position.x + area.size.x);
    std::uniform_real_distribution<float> yDist(area.position.y, area.position.y + area.size.y);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> lifeDist(1.0f, 4.0f);
    std::uniform_real_distribution<float> driftX(-3.0f, 3.0f);
    std::uniform_real_distribution<float> driftY(-5.0f, -1.0f);

    while ((int)m_particles.size() < targetCount) {
        Particle p;
        p.position = {xDist(rng), yDist(rng)};
        p.velocity = {driftX(rng), driftY(rng)};
        p.color = sf::Color(180, 200, 255, 100);
        p.life = lifeDist(rng);
        p.maxLife = p.life;
        p.size = sizeDist(rng);
        m_particles.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : m_particles) {
        p.position += p.velocity * dt;
        p.life -= dt;
    }

    // Swap-and-pop: move dead particles to end, erase in O(n)
    auto dead = std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle& p) { return p.life <= 0.0f; });
    m_particles.erase(dead, m_particles.end());
}

void ParticleSystem::render(sf::RenderTarget& target) const {
    if (m_particles.empty()) return;

    // Each particle → 2 triangles (6 vertices) for variable-size quad
    sf::VertexArray va(sf::PrimitiveType::Triangles, m_particles.size() * 6);
    for (size_t i = 0; i < m_particles.size(); ++i) {
        const auto& p = m_particles[i];
        float half = p.size / 2.0f;
        float x = p.position.x, y = p.position.y;

        // Fade alpha based on remaining life
        float t = std::max(0.0f, p.life / p.maxLife);
        sf::Color c = p.color;
        c.a = static_cast<std::uint8_t>(c.a * t);

        // Quad: 2 triangles, 6 vertices
        size_t vi = i * 6;
        va[vi + 0] = sf::Vertex({x - half, y - half}, c);
        va[vi + 1] = sf::Vertex({x + half, y - half}, c);
        va[vi + 2] = sf::Vertex({x - half, y + half}, c);
        va[vi + 3] = sf::Vertex({x + half, y - half}, c);
        va[vi + 4] = sf::Vertex({x + half, y + half}, c);
        va[vi + 5] = sf::Vertex({x - half, y + half}, c);
    }
    target.draw(va);
}

void ParticleSystem::clear() {
    m_particles.clear();
}
