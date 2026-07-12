#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

class ParticleSystem {
public:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color;
        float life = 0.0f;
        float maxLife = 1.0f;
        float size = 2.0f;
    };

    ParticleSystem() = default;
    ~ParticleSystem() = default;

    // burst: spawn count particles at pos with randomized params
    void emit(const sf::Vector2f& pos, int count, const sf::Color& c,
              float spdMin, float spdMax,
              float lifeMin, float lifeMax,
              float sizeMin, float sizeMax);

    // continuous: maintain targetCount particles in area (for background stars)
    void spawnStars(float dt, const sf::FloatRect& area, int targetCount);

    void update(float dt);
    void render(sf::RenderTarget& target) const;
    void clear();

private:
    std::vector<Particle> m_particles;
};
