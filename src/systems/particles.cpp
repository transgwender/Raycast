#include "particles.hpp"
#include "registry.hpp"
#include "render.hpp"

void ParticleSystem::init() {
    // initialize rng
    rng = std::default_random_engine(std::random_device()());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // particle test
    // auto light_texture = texture_manager.getVirtual("light");
    // auto smoke_texture = texture_manager.getVirtual("smoke");
    // for (int i = 0; i < 300000; i++) {
    //     Particle p;
    //     if (dist(rng) < 0.0f) {
    //         p.scale = vec2(4.0f);
    //         p.texture = light_texture;
    //     } else {
    //         p.scale = vec2(6.0f);
    //         p.texture = smoke_texture;
    //     }
    //     p.position = vec2(320, 180) * vec2(dist(rng), dist(rng));
    //     p.color = vec4(dist(rng), dist(rng), dist(rng), 0.5);
    //     p.angle = 0.0f;
    //     p.linear_velocity = vec2(1) * vec2(dist(rng), dist(rng)) * 40.0f;
    //     p.spin_velocity = 0.0f;
    //     p.scale_change = 0.0f;
    //     p.alpha_fall_off = 0.0f;
    //     p.lifetime = 50.0;
    //     registry.particles.insert(Entity(), p);
    // }

    // sprite particle test
    // for (int i = 0; i < 3000; i++) {
    //     auto position = vec2(320 * dist(rng), 180 * dist(rng));
    //     auto color = vec4(dist(rng), dist(rng), dist(rng), 1) / 2.f;
    //     auto linear_velocity = vec2(1) * vec2(dist(rng), dist(rng)) * 40.0f;
    //     const auto entity = createSprite(Entity(), position, {4.0, 4.0}, 0.0, "light", FOREGROUND, color);
    //     registry.motions.get(entity).velocity = linear_velocity;
    // }

    // particle spawner test
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
    // spawner.lifetime = 10.0;
    // spawner.max_particles = 4;
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
        particle.scale.x = particle.scale.x < 0.0f ? 0.0f : particle.scale.x;
        particle.scale.y = particle.scale.y < 0.0f ? 0.0f : particle.scale.y;
        particle.position += particle.scale_change * delta_time * -1 / 2;
        particle.angle += particle.spin_velocity * delta_time;
        particle.position += particle.linear_velocity * delta_time;
    }
}
