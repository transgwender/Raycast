// internal
#include "physics.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates
// scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion) {
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding
// boxes and sees if the center point of either object is inside the other's
// bounding-box-circle. You can surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2) {
    // TODO: we probably want to replace this with our own version
    vec2 dp = motion1.position - motion2.position;
    float dist_squared = dot(dp, dp);
    const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
    const float other_r_squared = dot(other_bonding_box, other_bonding_box);
    const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
    const float my_r_squared = dot(my_bonding_box, my_bonding_box);
    const float r_squared = max(other_r_squared, my_r_squared);
    if (dist_squared < r_squared)
        return true;
    return false;
}

/**
 * Advance the physics simulation by one step
 */
void PhysicsSystem::step(float elapsed_ms) {

    auto& motion_registry = registry.motions;
    float collectiveTime = 0;
    for (uint i = 0; i < motion_registry.size(); i++) {

        // motion.velocity
        Motion& motion = motion_registry.components[i];
        Entity entity = motion_registry.entities[i];
        float step_seconds = elapsed_ms / 1000.f;

        motion.position.x += step_seconds * motion.velocity.x;
        motion.position.y += step_seconds * motion.velocity.y;

        (void)elapsed_ms; // placeholder to silence unused warning until
                          // implemented
    }
}