#include "systems/physics.hpp"

#include "collisions.h"
#include "logging/log.hpp"
#include "utils/math.hpp"
#include "utils/time.hpp"
#include "world_init.hpp"
#include <climits>
#include <iostream>


/// PRIVATE declarations

void __updateVelocityFromBlackholePull(Motion &blackhole_motion, Motion &light_motion, float blackhole_mass);

/// End of PRIVATE declarations

// NOTE: of course these values deviate from the "real world" values and have been scaled to make sense in our little light maze 
// world -- for our purposes they lead to realistic behaviour
const float PhysicsSystem::GravitationalConstant = 8;
const float PhysicsSystem::SpeedOfLight = 100;
const float PhysicsSystem::MaxOrbitDistance = 30;
const float PhysicsSystem::MaxOrbitAngle = 5 * M_PI_2 / 12; // RADIANS
const float PhysicsSystem::MaxAngleToTravel = 3 * M_PI / 4;


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
    if (!shouldStep()) return;
    exert_blackhole_pull(elapsed_ms);
    update_positions(elapsed_ms);
}

void PhysicsSystem::update_positions(float elapsed_ms) {
    const float t = elapsed_ms / raycast::time::ONE_SECOND_IN_MS;
    auto& motion_registry = registry.motions;

    for (uint i = 0; i < motion_registry.size(); i++) {
        Motion& motion = motion_registry.components[i];
        Entity& entity = motion_registry.entities[i];
        motion.position.x += t * motion.velocity.x;
        motion.position.y += t * motion.velocity.y;
        // if motion entity has a collider, require an update if entity moved
        if (registry.colliders.has(entity) &&
            (dot(motion.velocity, motion.velocity) > 0 || motion.angle != registry.colliders.get(entity).angle)) {
            registry.colliders.get(entity).needs_update = true;
        }
    }
}

void PhysicsSystem::exert_blackhole_pull(float elapsed_ms) {
    const float t = elapsed_ms / raycast::time::ONE_SECOND_IN_MS;
    // exert pull towards blackhole(s)
    for (uint i = 0; i < registry.blackholes.size(); i++) {
        // NOTE: Blackholes exert a force across the entire level -- this may be refactored by having a radius inside
        // which a force should be exerted.
        //       We opted against this since the pulling force is inversely proportional to the distance between the
        //       light and the blackhole -- so if the light is really far away, the force on it is negligible. Also note
        //       that the blackhole only exerts a force on the light and nothing else.

        Entity blackhole_entity = registry.blackholes.entities[i];
        Blackhole& blackhole = registry.blackholes.components[i];
        Motion& blackhole_motion = registry.motions.get(blackhole_entity);

        uint light_rays_count = registry.lightRays.size();
        for (uint j = 0; j < light_rays_count; j++) {
            Entity light_ray_entity = registry.lightRays.entities[j];
            Motion& light_ray_motion = registry.motions.get(light_ray_entity);

            if (should_light_orbit(light_ray_entity, blackhole_entity) && !registry.inOrbits.has(light_ray_entity)) {
                startLightOrbit(light_ray_entity, light_ray_motion, blackhole_motion, blackhole_entity);
            }

            if (registry.inOrbits.has(light_ray_entity) && registry.inOrbits.get(light_ray_entity).bodyOfMass == blackhole_entity) {
                updateVelocityDuringOrbit(light_ray_entity, light_ray_motion, blackhole_motion, t);

            } else {
                int retFlag;
                updateVelocityFromBlackholePull(light_ray_entity, blackhole_entity, blackhole_motion, light_ray_motion,
                                                blackhole, retFlag);
                if (retFlag == 3)
                    continue;
            }
        }
    }

    // End zone are really, really small blackholes too! This is to make it more enjoyable to play levels (you don't need to get the angles) 
    // _exactly_ right to win the level and it also serves as a great animation.
    // NOTE: we do not add the blackhole component to the end zone directly since blackhole components have their own shader code that we don't want 
    // to apply to the endzone
    for (int i = 0; i < registry.zones.size(); i++) {
        Zone &zone = registry.zones.components[i];
        if (zone.type == ZONE_TYPE::END) {
            uint light_rays_count = registry.lightRays.size();
            for (uint j = 0; j < light_rays_count; j++) {
                Entity light_ray_entity = registry.lightRays.entities[j];
                Motion& light_ray_motion = registry.motions.get(light_ray_entity);
                // come to me my dear light
                Entity endzone_entity = registry.zones.entities[i];
                Motion blackhole_motion;
                blackhole_motion.position = zone.position;
                // only apply the endzone gravitational pull when the light is near the end zone 
                vec2 to_light = light_ray_motion.position - blackhole_motion.position;
                if (glm::length(to_light) < zone.force_field_radius) {
                    __updateVelocityFromBlackholePull(blackhole_motion, light_ray_motion, zone.mass);
                }
            }
        }
    }
}

void PhysicsSystem::updateVelocityFromBlackholePull(Entity& light_ray_entity, Entity& blackhole_entity,
                                                    Motion& blackhole_motion, Motion& light_ray_motion,
                                                    Blackhole& blackhole, int& retFlag) {
    retFlag = 1;
    // Now, these aren't very _realistic_ calculations in that they do not model relativity and all that
    // stuff
    // that a really smart guy named Albert Einstein worked on. But they do try to mimic relativity and are
    // not as naive as a Newtonian model of a blackhole. This means that the path that the light follows is
    // actually fairly realistic (it respects the ratios of the schwarzchild radius at which light should be
    // sucked in vs when it should escape without having to cheat and hardcode them!). The formulas we used
    // are credited to Chris Orban from STEMCoding. See this link:
    // https://www.asc.ohio-state.edu/orban.14/stemcoding/blackhole.html

    // TODO: Summarize math in README.md for Peyton
    if (registry.inOrbits.has(light_ray_entity) &&
        registry.inOrbits.get(light_ray_entity).bodyOfMassJustOrbited == blackhole_entity) {
        {
            retFlag = 3;
            return;
        }; // This is a temp fix so that the black hole doesn't influence the light ray once it is released.
    }
    __updateVelocityFromBlackholePull(blackhole_motion, light_ray_motion, blackhole.mass);
}

/// @brief updates the light_motion based on the blackhole with motion blackhole_motiona and mass blackhole_mass
void __updateVelocityFromBlackholePull(Motion &blackhole_motion, Motion &light_ray_motion, float blackhole_mass) {
    vec2 to_blackhole = blackhole_motion.position - light_ray_motion.position;
    float theta = raycast::math::heading(to_blackhole);
    float distance_to_blackhole = glm::length(to_blackhole);
    float force_gravity =
        PhysicsSystem::GravitationalConstant * blackhole_mass / (distance_to_blackhole * distance_to_blackhole);
    float delta_theta = -force_gravity * (15.0 / PhysicsSystem::SpeedOfLight) * ::sin(light_ray_motion.angle - theta);
    delta_theta /=
        std::abs(1.0 - 2.0 * PhysicsSystem::GravitationalConstant * blackhole_mass /
                (distance_to_blackhole * PhysicsSystem::SpeedOfLight * PhysicsSystem::SpeedOfLight));
    light_ray_motion.angle += delta_theta;
    light_ray_motion.velocity = raycast::math::from_angle(light_ray_motion.angle);
    light_ray_motion.velocity = raycast::math::set_mag(light_ray_motion.velocity, PhysicsSystem::SpeedOfLight);
}

void PhysicsSystem::updateVelocityDuringOrbit(Entity& light_ray_entity, Motion& light_ray_motion,
        Motion& blackhole_motion, const float t) {
    InOrbit& in_orbit = registry.inOrbits.get(light_ray_entity);
    vec2 relativePosition = light_ray_motion.position - blackhole_motion.position;

    // Updating the velocity of the light particle to orbit around the black hole
    float det = relativePosition.x * light_ray_motion.velocity.y - relativePosition.y * light_ray_motion.velocity.x;

    vec2 tangent =
        det > 0 ? vec2(-relativePosition.y, relativePosition.x) : vec2(relativePosition.y, -relativePosition.x);
    light_ray_motion.velocity = tangent / glm::length(tangent) * glm::length(light_ray_motion.velocity);

    // Updating the total angle travelled
    float radius = glm::length(relativePosition);
    float delta_angle = t * glm::length(light_ray_motion.velocity) / radius;
    in_orbit.totalAngle += delta_angle;

    if (in_orbit.totalAngle >= PhysicsSystem::MaxAngleToTravel) {
        in_orbit.bodyOfMassJustOrbited = in_orbit.bodyOfMass;
        in_orbit.bodyOfMass = Entity(); // this does bother me :(
    }
}

void PhysicsSystem::startLightOrbit(Entity& light_ray_entity, Motion& light_ray_motion, Motion& blackhole_motion,
                                    Entity& blackhole_entity) {
    if (!registry.inOrbits.has(light_ray_entity)) {
        InOrbit in_orbit;
        vec2 relativePosition = light_ray_motion.position - blackhole_motion.position;
        in_orbit.prevAngle = atan2(relativePosition.y, relativePosition.x);
        in_orbit.bodyOfMass = blackhole_entity;
        registry.inOrbits.emplace(light_ray_entity, in_orbit);
    } else if (registry.inOrbits.get(light_ray_entity).bodyOfMassJustOrbited != blackhole_entity) {
        InOrbit& in_orbit = registry.inOrbits.get(light_ray_entity);
        vec2 relativePosition = light_ray_motion.position - blackhole_motion.position;
        in_orbit.prevAngle = atan2(relativePosition.y, relativePosition.x);
        in_orbit.bodyOfMass = blackhole_entity;
    }
}

bool PhysicsSystem::should_light_orbit(Entity light, Entity blackhole) {
    Motion& light_motion = registry.motions.get(light);
    Motion& blackhole_motion = registry.motions.get(blackhole);
    vec2 displacement = blackhole_motion.position - light_motion.position;
    float distance_to_blackhole = glm::length(displacement);

    if (distance_to_blackhole > PhysicsSystem::MaxOrbitDistance) {
        return false;
    }

    // We calculate both tangents to account for all directions that the light ray could be travelling

    vec2 left_tangent = vec2(-displacement.y, displacement.x);
    vec2 right_tangent = vec2(displacement.y, -displacement.x);
    vec2 velocity = light_motion.velocity;

    float left_length_product = glm::length(left_tangent) * glm::length(velocity);
    float left_dot_product = dot(left_tangent, velocity);
    float left_angle = acos(left_dot_product / left_length_product);

    float right_length_product = glm::length(right_tangent) * glm::length(velocity);
    float right_dot_product = dot(right_tangent, velocity);
    float right_angle = acos(right_dot_product / right_length_product);

    return (left_angle <= PhysicsSystem::MaxOrbitAngle || right_angle <= PhysicsSystem::MaxOrbitAngle);
}

// check for collisions between entities that collide
void PhysicsSystem::detect_collisions() {
    ComponentContainer<Collideable>& collideable_registry = registry.collideables;
    for (uint i = 0; i < collideable_registry.components.size(); i++) {
        Entity entity_i = collideable_registry.entities[i];
        if (registry.invisibles.has(entity_i)) {
            continue;
        }
        // start collision detection from next entity (to avoid self-, repeated-comparisons)
        for (uint j = i + 1; j < collideable_registry.components.size(); j++) {
            Entity entity_j = collideable_registry.entities[j];
            if (registry.invisibles.has(entity_j)) {
                continue;
            }
            // create a collisions event for each entity colliding with other
            // (to ensure both orders exist for later collision handling)
            vec2 collision = Collisions::overlap(entity_i, entity_j);
            if (collision.x != 0) {
                // LOG_INFO("Collision detected\n");
                try {
                    Collision& c1 = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                    Collision& c2 = registry.collisions.emplace_with_duplicates(entity_j, entity_i);

                    c1.side = collision.x;
                    c2.side = collision.x;
                    c1.overlap = collision.y;
                    c2.overlap = collision.y;
                } catch (...) {
                    LOG_CRITICAL("Something BAD happened with Collisions");
                }



            }
        }
    }
}
