#include "systems/physics.hpp"
#include "logging/log.hpp"
#include "utils/math.hpp"
#include "world_init.hpp"
#include <climits>
#include <iostream>

const float ONE_SECOND = 1000.f;

/*
    Linear interpolation algorithm

    @param start: The starting value of the interpolation.
    @param end: The ending value of the interpolation.
    @param t: A float value between 0 and 1 representing the interpolation factor.
              - 0 will return the start value.
              - 1 will return the end value.
              - Values between 0 and 1 will return intermediate values.
*/
float lerp(float start, float end, float t) {
    return start * (1 - t) + (end * t);
}

// Returns the local bounding coordinates (bottom left and top right)
// scaled by the current size of the entity
// rotated by the entity's current rotation, relative to the origin
std::array<vec2, 4> get_bounding_points(const Motion& motion) {
    // get vectors to top right of origin-located bounding rectangle
    vec2 top_right = vec2(abs(motion.scale.x) / 2.f, abs(motion.scale.y) / 2.f);
    vec2 bottom_right{top_right.x, -top_right.y};
    vec2 bottom_left{-top_right.x, -top_right.y};
    vec2 top_left{-top_right.x, top_right.y};

    // calculate rotation matrix
    float cos = ::cos(motion.angle);
    float sin = ::sin(motion.angle);
    mat2 rotation_matrix = mat2(cos, sin, -sin, cos);

    // rotate points about the origin by motion's angle
    top_right = rotation_matrix * top_right;
    bottom_right = rotation_matrix * bottom_right;
    bottom_left = rotation_matrix * bottom_left;
    top_left = rotation_matrix * top_left;
    // bottom left and top left corners of rectangle are
    // -top_right and -bottom_right respectively (by math)
    return {top_right, bottom_right, bottom_left, top_left};
}

// Returns the minimum and maximum magnitudes of points
// projected onto an axis as a vec2
vec2 get_projected_min_max(const std::array<vec2, 4>& bounding_points,
						   const vec2 axis) {
	vec2 min_max = {INT_MAX, INT_MIN};
	for (const vec2& point : bounding_points) {
		float projected = dot(point, axis);
		min_max = {min(projected, min_max.x), max(projected, min_max.y)};
	}
	return min_max;
}

// Returns true if neither vector has a component between those of the
// other vector, implying no overlap
// Invariant: assume the vectors contain minimum and maximum
// values for points projected onto an axis in their x and y
// fields respectively
bool no_overlap(std::array<vec2, 4> m1_bounding_points,
				std::array<vec2, 4> m2_bounding_points,
				float angle) {
	vec2 axis = {::cos(angle), ::sin(angle)};
	vec2 m1_min_max = get_projected_min_max(m1_bounding_points, axis);
	vec2 m2_min_max = get_projected_min_max(m2_bounding_points, axis);
	return m1_min_max.x > m2_min_max.y || m1_min_max.y < m2_min_max.x;
}

// Returns true if motion1 and motion2 are overlapping using a coarse
// step with radial boundaries and a fine step with the separating axis theorem
bool collides(const Motion& motion1, const Motion& motion2) {
    // see if the distance between centre points of motion1 and motion2
    // are within the maximum possible distance for them to be touching
    vec2 dp = motion1.position - motion2.position;
    float dist_squared = dot(dp, dp);
    float max_possible_collision_distance =
        (dot(motion1.scale, motion1.scale) +
         dot(motion2.scale, motion2.scale)) /
        2.f;
    // radial boundary-based estimate
    if (dist_squared < max_possible_collision_distance) {
        LOG_INFO("Collision possible. Refining...");
        // move points to their screen location
        std::array<vec2, 4> m1_bounding_points = get_bounding_points(motion1);
        for (vec2& point : m1_bounding_points) {
            point += motion1.position;
        }
        std::array<vec2, 4> m2_bounding_points = get_bounding_points(motion2);
        for (vec2& point : m2_bounding_points) {
            point += motion2.position;
        }
        // define all axis angles (normals to edges)
        float axis_angles[4];
        axis_angles[0] = motion1.angle;
        axis_angles[1] = M_PI_2 + motion1.angle;
        axis_angles[2] = motion2.angle;
        axis_angles[3] = M_PI_2 + motion2.angle;
        // see if an overlap exists in any of the axes
        for (float& angle : axis_angles) {
            if (no_overlap(m1_bounding_points, m2_bounding_points, angle)) {
                LOG_INFO("No collision!");
                return false;
            }
        }
        LOG_INFO("Collision detected between motion with position ({}, {}) and "
                 "motion with position ({}, {})",
                 motion1.position.x, motion1.position.y, motion2.position.x,
                 motion2.position.y);
        return true;
    }
    return false;
}

/**
 * Advance the physics simulation by one step
 */
void PhysicsSystem::step(float elapsed_ms) {
    auto& motion_registry = registry.motions;

    for (uint i = 0; i < motion_registry.size(); i++) {
        Motion& motion = motion_registry.components[i];
        float t = elapsed_ms / ONE_SECOND;
        motion.position.x += t * motion.velocity.x;
        motion.position.y += t * motion.velocity.y;
    }

    // Step all entities on rails
    auto& linear_rails_registry = registry.entitiesOnLinearRails;
    for (uint i = 0; i < linear_rails_registry.size(); i++) {
        auto e = linear_rails_registry.entities[i];
        OnLinearRails& r = linear_rails_registry.components[i];
        Motion& m = registry.motions.get(e);
        LinearlyInterpolatable& lr = registry.linearlyInterpolatables.get(e);
        float t = elapsed_ms / ONE_SECOND;
        if (lr.should_switch_direction) {
            lr.t += t * lr.t_step;
        } else {
            lr.t -= t * lr.t_step;
        }
        if (raycast::math::definitelyGreaterThan(lr.t, 1.0)) {
            lr.should_switch_direction = false;
        } else if (raycast::math::definitelyLessThan(lr.t, 0.0)) {
            lr.should_switch_direction = true;
        }
        m.position =
            raycast::math::lerp(r.firstEndpoint, r.secondEndpoint, lr.t);
    }

    // check for collisions between entities that collide
    ComponentContainer<Motion> &motion_container = registry.motions;
    for(uint i = 0; i<motion_container.components.size(); i++)
    {
        Motion& motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];
		if (motion_i.collides) {
		    // start collision detection from next entity (to avoid self-, repeated-comparisons)
		    for(uint j = i+1; j<motion_container.components.size(); j++) {
		        Motion& motion_j = motion_container.components[j];
		        if (motion_j.collides && collides(motion_i, motion_j))
		        {
		            Entity entity_j = motion_container.entities[j];
		            // create a collisions event for each entity colliding with other
		            // (to ensure both orders exist for later collision handling)
		             // NOTE: stubbed with REFLECTIVE collisions for now
                    std::cout << "COLLISION!" << std::endl;
		            std::cout <<entity_i << ", " << entity_j << std::endl;
		            registry.collisions.emplace_with_duplicates(entity_i, entity_j);
		            registry.collisions.emplace_with_duplicates(entity_j, entity_i);
		        }
		    }
		}
    }
}
