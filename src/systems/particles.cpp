#include "particles.hpp"
#include "registry.hpp"
#include "render.hpp"

void ParticleSystem::init() {
    ParticleSpawner spawner;
    spawner.texture = texture_manager.get("smoke");
    spawner.position = vec2(160, 80);
    spawner.initial_speed = 5.0f;
    spawner.spin_velocity = 0.0f;
    spawner.direction = vec2(0, 1);
    LOG_INFO("direction angle: {}", atan2(1, 0) + M_PI_4);
    spawner.color = vec4(255, 200, 0, 255);
    spawner.spread = M_PI_2;
    spawner.scale_fall_off = 0.f;
    spawner.lifetime = 2.0;
    spawner.max_particles = 4;
    registry.particleSpawners.insert(Entity(), spawner);
}

void ParticleSystem::step(float elapsed_ms) {
    const float delta_time = elapsed_ms / 1000.0f;

    for (ParticleSpawner& spawner : registry.particleSpawners.components) {
        spawner._cooldown -= delta_time;
        if (spawner._cooldown <= 0.0f) {
            spawner._cooldown = spawner.lifetime / spawner.max_particles;

            // cooldown is up, spawn a new particle
            Particle p;
            p.texture = spawner.texture;
            p.position = spawner.position;
            p.color = spawner.color;
            p.scale = vec2(32, 32);
            p.angle = 0.0f;
            p.linear_velocity = {0.0f, 40.0f};
            p.spin_velocity = spawner.spin_velocity;
            p.scale_fall_off = spawner.scale_fall_off;
            p.lifetime = spawner.lifetime;
            registry.particles.insert(Entity(), p);
        }
    }

    for (int i = 0; i < registry.particles.size(); i++) {
        const Entity& particle_entity = registry.particles.entities[i];
        Particle& particle = registry.particles.components[i];
        particle.lifetime -= delta_time;
        if (particle.lifetime <= 0.0f) {
            registry.particles.remove(particle_entity);
            continue;
        }
        particle.scale += particle.scale_fall_off * delta_time;
        particle.angle += particle.spin_velocity * delta_time;
        particle.position += particle.scale_fall_off * delta_time * -1 / 2;
        particle.position += particle.linear_velocity * delta_time;
    }
}
