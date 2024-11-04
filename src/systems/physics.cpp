#include "systems/physics.hpp"

#include "collisions.h"
#include "logging/log.hpp"
#include "utils/math.hpp"
#include "utils/time.hpp"
#include "world_init.hpp"
#include <climits>
#include <iostream>

// NOTE: of course these values deviate from the "real world" values and have been scaled to make sense in our little light maze 
// world -- for our purposes they lead to realistic behaviour
const float PhysicsSystem::GravitationalConstant = 8;
const float PhysicsSystem::SpeedOfLight = 100;

bool PhysicsSystem::shouldStep() {
    assert(registry.menus.size() <= 1);
    if (!registry.menus.components.empty()) {
        return !registry.menus.components.front().shouldBlockSteps;
    }
    return true;
}

/**
 * Advance the physics simulation by one step
 */
void PhysicsSystem::step(float elapsed_ms) {
    if(!shouldStep()) return;

    const float t = elapsed_ms / raycast::time::ONE_SECOND_IN_MS;
    auto& motion_registry = registry.motions;
    ComponentContainer<Collideable>& collideable_registry = registry.collideables;

    for (uint i = 0; i < motion_registry.size(); i++) {
        Motion& motion = motion_registry.components[i];
        Entity& entity = motion_registry.entities[i];
        motion.position.x += t * motion.velocity.x;
        motion.position.y += t * motion.velocity.y;
        // if motion entity has a collider, require an update if entity moved
        if (registry.colliders.has(entity)
            && (dot(motion.velocity, motion.velocity) > 0
                || motion.angle != registry.colliders.get(entity).angle)) {
            registry.colliders.get(entity).needs_update = true;
        }
    }

    // check for collisions between entities that collide
    for (uint i = 0; i < collideable_registry.components.size(); i++) {
        Entity entity_i = collideable_registry.entities[i];
        // start collision detection from next entity (to avoid self-, repeated-comparisons)
        for (uint j = i + 1; j < collideable_registry.components.size(); j++) {
            Entity entity_j = collideable_registry.entities[j];
            // create a collisions event for each entity colliding with other
            // (to ensure both orders exist for later collision handling)
            int collision = Collisions::collides(entity_i, entity_j);
            if (collision > 0) {
                // LOG_INFO("Collision detected\n");
                registry.collisions.emplace_with_duplicates(entity_i, entity_j).side = collision;
                registry.collisions.emplace_with_duplicates(entity_j, entity_i).side = collision;
            }
        }
    }

    // exert pull towards blackhole(s)
    for (uint i = 0; i < registry.blackholes.size(); i++) {
        // NOTE: Blackholes exert a force across the entire level -- this may be refactored by having a radius inside which a force should be exerted.
        //       We opted against this since the pulling force is inversely proportional to the distance between the light and the blackhole -- so if
        //       the light is really far away, the force on it is negligible. Also note that the blackhole only exerts a force on the light and nothing else.

        Entity blackhole_entity = registry.blackholes.entities[i];
        Blackhole &blackhole = registry.blackholes.components[i];
        Motion &blackhole_motion = registry.motions.get(blackhole_entity);

        uint light_rays_count = registry.lightRays.size();
        for (uint j = 0; j < light_rays_count; j++) {
            Entity light_ray_entity = registry.lightRays.entities[j];
            Motion &light_ray_motion = registry.motions.get(light_ray_entity);

            // Now, these aren't very _realistic_ calculations in that they do not model relativity and all that stuff that a really smart guy named Albert Einstein
            // worked on. But they do try to mimic relativity and are not as naive as a Newtonian model of a blackhole. This means that the path that the light follows is 
            // actually fairly realistic (it respects the ratios of the schwarzchild radius at which light should be sucked in vs when it should escape without having to
            // cheat and hardcode them!). The formulas we used are credited to Chris Orban from STEMCoding. See this link: https://www.asc.ohio-state.edu/orban.14/stemcoding/blackhole.html

            // TODO: Summarize math in README.md for Peyton
            vec2 to_blackhole = blackhole_motion.position - light_ray_motion.position;
            float theta = raycast::math::heading(to_blackhole);
            float distance_to_blackhole = glm::length(to_blackhole);
            float force_gravity = PhysicsSystem::GravitationalConstant * blackhole.mass / (distance_to_blackhole * distance_to_blackhole);
            float delta_theta = -force_gravity * (15.0 / PhysicsSystem::SpeedOfLight) * ::sin(light_ray_motion.angle - theta);
            delta_theta /= std::abs(1.0 - 2.0 * PhysicsSystem::GravitationalConstant * blackhole.mass / (distance_to_blackhole * PhysicsSystem::SpeedOfLight * PhysicsSystem::SpeedOfLight));
            light_ray_motion.angle += delta_theta;
            light_ray_motion.velocity = raycast::math::from_angle(light_ray_motion.angle);
            light_ray_motion.velocity = raycast::math::set_mag(light_ray_motion.velocity, PhysicsSystem::SpeedOfLight);
        }
    }
}
