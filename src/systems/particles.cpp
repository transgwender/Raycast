#include "particles.hpp"

#include "ai.hpp"
#include "registry.hpp"
#include "render.hpp"

void ParticleSystem::init() {
    // initialize rng
    rng = std::default_random_engine(std::random_device()());
    uniform_dist = std::uniform_real_distribution<float>(0.0f, 1.0f);

    // particle test
    // ParticleSpawner spawner;
    // spawner.texture = texture_manager.get("white_circle");
    // spawner.position = vec2(160, 50);
    // spawner.initial_speed = 300.0f;
    // spawner.damping = 2500.0f;
    // spawner.spin_velocity = 0.0f;
    // spawner.direction = vec2(1, 0);
    // spawner.color = vec4(1, 1, 0, 1);
    // spawner.spread = 2 * M_PI;
    // spawner.initial_scale = vec2(3, 3);
    // spawner.scale_change = -12.0f;
    // spawner.alpha_change = -4.0f;
    // spawner.lifetime = 0.25f;
    // spawner.max_particles = 10;
    // spawner.explosive = true;
    // spawner.uniform_explosion = true;
    // spawner.explosion_interval = 1.0f;
    //
    // Motion m;
    // m.scale = vec2(8.f);
    // m.position = spawner.position;
    //
    // auto e = Entity();
    // registry.particleSpawners.insert(e, spawner);
    // registry.motions.insert(e, m);
}

void ParticleSystem::step(float elapsed_ms) {
    const float delta_time = elapsed_ms / 1000.0f;

    for (int i = 0; i < registry.particleSpawners.size(); i++) {
        ParticleSpawner& spawner = registry.particleSpawners.components[i];
        const Entity& spawner_entity = registry.particleSpawners.entities[i];

        spawner.time_to_live -= delta_time;
        if (spawner.time_to_live <= 0.0f) {
            registry.particleSpawners.remove(spawner_entity);
            continue;
        }

        const Motion& motion = registry.motions.get(spawner_entity);
        spawner.position = motion.position - (motion.scale / 2.0f);

        spawner._cooldown -= delta_time;
        if (spawner._cooldown <= 0.0f) {
            if (spawner.explosive) {
                spawner._cooldown = spawner.explosion_interval;
            } else {
                spawner._cooldown = spawner.lifetime / spawner.max_particles;
            }

            int num_particles = spawner.explosive ? spawner.max_particles : 1;

            for (int j = 0; j < num_particles; j++) {
                float rotation_angle;
                // choose a direction
                if (spawner.explosive && spawner.uniform_explosion) {
                    rotation_angle = (spawner.spread * (static_cast<float>(j) / num_particles)) - (spawner.spread / 2);
                } else {
                    rotation_angle = (spawner.spread * uniform_dist(rng)) - (spawner.spread / 2);
                }

                float new_x = (spawner.direction.x * cos(rotation_angle)) - (spawner.direction.y * sin(rotation_angle));
                float new_y = (spawner.direction.x * sin(rotation_angle)) + (spawner.direction.y * cos(rotation_angle));
                auto particle_direction = vec2(new_x, new_y);

                // cooldown is up, spawn a new particle
                Particle p;
                p.texture = spawner.texture;
                p.position = spawner.position;
                p.color = spawner.color;
                p.scale = spawner.initial_scale;
                p.angle = 0.0f;
                p.direction = particle_direction;
                p.speed = spawner.initial_speed;
                p.damping = spawner.damping;
                p.spin_velocity = spawner.spin_velocity;
                p.scale_change = spawner.scale_change;
                p.alpha_fall_off = spawner.alpha_change;
                p.lifetime = spawner.lifetime;
                registry.particles.insert(Entity(), p);
            }
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

        particle.color.a += particle.alpha_fall_off * delta_time;

        particle.scale += particle.scale_change * delta_time;
        particle.scale.x = particle.scale.x < 0.0f ? 0.0f : particle.scale.x;
        particle.scale.y = particle.scale.y < 0.0f ? 0.0f : particle.scale.y;
        particle.position += particle.scale_change * delta_time * -1 / 2;

        particle.angle += particle.spin_velocity * delta_time;

        particle.speed -= particle.damping * delta_time;
        particle.speed = particle.speed < 0.0f ? 0.0f : particle.speed;
        particle.position += particle.speed * particle.direction * delta_time;
    }
}

Entity ParticleSystem::createLightDissipation(const Motion& light_motion) {
    ParticleSpawner spawner;
    spawner.texture = texture_manager.get("white_circle");
    spawner.position = (light_motion.position + (light_motion.scale / 2.0f)) - 1.0f;
    spawner.time_to_live = 1.0f;
    spawner.initial_speed = 300.0f;
    spawner.damping = 3500.0f;
    spawner.spin_velocity = 0.0f;
    spawner.direction = vec2(1, 0);
    spawner.color = vec4(1, 1, 0, 1);
    spawner.spread = 2 * M_PI;
    spawner.initial_scale = vec2(3, 3);
    spawner.scale_change = -12.0f;
    spawner.alpha_change = -4.0f;
    spawner.lifetime = 0.5f;
    spawner.max_particles = 10;
    spawner.explosive = true;
    spawner.uniform_explosion = true;
    spawner.explosion_interval = 1.0f;

    Motion m;
    m.scale = light_motion.scale;
    m.position = spawner.position;

    auto e = Entity();
    registry.particleSpawners.insert(e, spawner);
    registry.motions.insert(e, m);

    return e;
}

Entity ParticleSystem::createPortalParticles(const Portal& portal, const vec4& color) {
    ParticleSpawner spawner;
    spawner.texture = texture_manager.get("white_circle");
    spawner.position = portal.position + vec2(13, 42) / 2.0f;
    spawner.initial_speed = 300.0f;
    spawner.damping = 3000.0f;
    spawner.spin_velocity = 0.0f;
    spawner.direction = vec2(cos(portal.angle), sin(portal.angle));
    spawner.color = vec4(color.x / 255, color.y / 255, color.z / 255, color.w / 255);
    spawner.spread = M_PI / 2;
    spawner.initial_scale = vec2(3, 3);
    spawner.scale_change = -12.0f;
    spawner.alpha_change = 1.f;
    spawner.lifetime = 0.5f;
    spawner.max_particles = 8;

    Motion m;
    m.scale = vec2(13, 42);
    m.position = spawner.position;

    auto e = Entity();
    registry.particleSpawners.insert(e, spawner);
    registry.motions.insert(e, m);

    return e;
}

