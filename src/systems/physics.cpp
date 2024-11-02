#include "systems/physics.hpp"

#include "collisions.h"
#include "logging/log.hpp"
#include "systems/rails.hpp"
#include "utils/math.hpp"
#include "world_init.hpp"
#include <climits>
#include <iostream>

const float ONE_SECOND = 1000.f;

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

    const float t = elapsed_ms / ONE_SECOND;
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
                LOG_INFO("Collision detected\n");
                registry.collisions.emplace_with_duplicates(entity_i, entity_j).side = collision;
                registry.collisions.emplace_with_duplicates(entity_j, entity_i).side = collision;
            }
        }
    }
}
