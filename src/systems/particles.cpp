#include "particles.hpp"
#include "registry.hpp"
#include "render.hpp"

void ParticleSystem::init() {
    // registry.particles.insert(Entity(), {texture_manager.get("bubble_closed"), vec2(10, 10), vec2(12 ,12), 0.0f, vec2(10, 10)});
    for (int i = 0; i < 1; i++) {
        registry.particles.insert(Entity(), {texture_manager.get("bubble_closed"), vec2(i % 320, i % 180), vec2(4 ,4), i / 4.0f, vec2(i % 10, i % 20)});
    }
}

void ParticleSystem::step(float elapsed_ms) {
    const float delta_time = elapsed_ms / 1000.0f;
    for (Particle& particle : registry.particles.components) {
        particle.position += particle.velocity * delta_time;
    }
}
