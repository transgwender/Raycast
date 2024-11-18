#include "particles.hpp"
#include "registry.hpp"
#include "render.hpp"

void ParticleSystem::init() {
    // initialize rng
    rng = std::default_random_engine(std::random_device()());

    // ParticleSpawner spawner;
    // spawner.texture = texture_manager.get("light");
    // spawner.position = vec2(160, 80);
    // spawner.initial_speed = 5.0f;
    // spawner.spin_velocity = 0.0f;
    // spawner.direction = vec2(0, 1);
    // spawner.color = vec4(255, 255, 0, 255);
    // spawner.spread = M_PI_2;
    // spawner.initial_scale = vec2(8, 8);
    // spawner.scale_change = 0.f;
    // spawner.alpha_fall_off = 255.0f;
    // spawner.lifetime = 1.0;
    // spawner.max_particles = 4;
    // registry.particleSpawners.insert(Entity(), spawner);
}

void ParticleSystem::step(float elapsed_ms) {
    const float delta_time = elapsed_ms / 1000.0f;

    for (int i = 0; i < registry.particleSpawners.size(); i++) {
        ParticleSpawner& spawner = registry.particleSpawners.components[i];
        const Entity& spawner_entity = registry.particleSpawners.entities[i];
        const Motion& motion = registry.motions.get(spawner_entity);
        spawner.position = motion.position - (motion.scale / 2.0f);

        spawner._cooldown -= delta_time;
        if (spawner._cooldown <= 0.0f) {
            spawner._cooldown = spawner.lifetime / spawner.max_particles;

            // cooldown is up, spawn a new particle
            Particle p;
            p.texture = spawner.texture;
            p.position = spawner.position;
            p.color = spawner.color;
            p.scale = spawner.initial_scale;
            p.angle = 0.0f;
            p.linear_velocity = spawner.direction * spawner.initial_speed;
            p.spin_velocity = spawner.spin_velocity;
            p.scale_change = spawner.scale_change;
            p.alpha_fall_off = spawner.alpha_fall_off;
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
        particle.color.a -= particle.alpha_fall_off * delta_time;
        particle.scale += particle.scale_change * delta_time;
        particle.scale = max(particle.scale, 0.0f);
        particle.position += particle.scale_change * delta_time * -1 / 2;
        particle.angle += particle.spin_velocity * delta_time;
        particle.position += particle.linear_velocity * delta_time;
    }
}
