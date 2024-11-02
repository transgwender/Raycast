#include "collisions.h"

#include "registry.hpp"
#include "utils/math.hpp"

#include <array>

struct Collider;
struct Motion;
// Returns the local bounding coordinates (bottom left and top right)
// rotated by the entity's current rotation, relative to the origin
std::array<vec2, 4> get_bounding_points(Collider& collider, const Entity& entity) {
    // update collider bounds if needed
    if (collider.needs_update) {
        Motion& motion = registry.motions.get(entity);
        // get vectors to top right of origin-located bounding rectangle
        vec2 top_right = vec2(abs(collider.width) / 2.f, abs(collider.height) / 2.f);
        vec2 bottom_right{top_right.x, -top_right.y};
        vec2 bottom_left{-top_right.x, -top_right.y};
        vec2 top_left{-top_right.x, top_right.y};
        // calculate rotation matrix
        float cos = ::cos(motion.angle);
        float sin = ::sin(motion.angle);
        mat2 rotation_matrix = mat2(cos, sin, -sin, cos);
        collider.rotated_bounds[0] = rotation_matrix * top_right;
        collider.rotated_bounds[1] = rotation_matrix * bottom_right;
        collider.rotated_bounds[2] = rotation_matrix * bottom_left;
        collider.rotated_bounds[3] = rotation_matrix * top_left;
        collider.angle = motion.angle;
        collider.needs_update = false;
    }

    return {collider.rotated_bounds[0],
        collider.rotated_bounds[1],
        collider.rotated_bounds[2],
        collider.rotated_bounds[3]};
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
// step with radial boundaries and different fine steps depending on collider types
// The optional user_interaction flag allows different boundaries to be used for
// interactions if set
// Returns 1 if collision along sprite y side of rectangle
// Returns 2 if collision along sprite x side of rectangle
int Collisions::collides(const Entity& e1, const Entity& e2, bool user_interaction) {
    // see if the distance between centre points of motion1 and motion2
    // are within the maximum possible distance for them to be touching
    Motion& motion1 = registry.motions.get(e1);
    Motion& motion2 = registry.motions.get(e2);
    Collider& collider1 = registry.colliders.get(e1);
    Collider& collider2 = registry.colliders.get(e2);
    BOUNDS_TYPE bounds_type1;
    BOUNDS_TYPE bounds_type2;
    float radius1 = max(collider1.width, collider1.height);
    float radius2 = max(collider2.width, collider2.height);
    if (user_interaction) {
        bounds_type1 = collider1.user_interaction_bounds_type;
        bounds_type2 = collider2.user_interaction_bounds_type;
    } else {
        bounds_type1 = collider1.bounds_type;
        bounds_type2 = collider2.bounds_type;
    }
    vec2 dp = motion1.position - motion2.position;
    float dist_squared = dot(dp, dp);
    float max_possible_collision_distance =
        (dot(vec2(collider1.width, collider1.height), vec2(collider1.width, collider1.height)) +
         dot(vec2(collider2.width, collider2.height), vec2(collider2.width, collider2.height))) /
        2.f;
    if (dist_squared < max_possible_collision_distance) {
        // LOG_INFO("Collision possible. Refining...");

        // for all point colliders, should have radius 0.5 so it is possible
        // to treat as radial without loss of functionality
        if (bounds_type1 == BOUNDS_TYPE::POINT) {
            collider1.width = 0.5f;
            collider1.height = 0.5f;
        }
        if (bounds_type2 == BOUNDS_TYPE::POINT) {
            collider2.width = 0.5f;
            collider2.height = 0.5f;
        }

        // Both colliders radial
        if ((bounds_type1 == BOUNDS_TYPE::RADIAL || bounds_type1 == BOUNDS_TYPE::POINT)
                && (bounds_type2 == BOUNDS_TYPE::RADIAL || bounds_type2 == BOUNDS_TYPE::POINT)) {
            if (dist_squared < max_possible_collision_distance/2.f) {
                LOG_INFO("Radial radial collision\n");
                return 1;
            }
            return 0;
        }

        std::array<vec2, 4> m2_bounding_points = get_bounding_points(collider2, e2);

        // One collider radial or point, one rectangular
        if ((bounds_type1 == BOUNDS_TYPE::RADIAL || bounds_type1 == BOUNDS_TYPE::POINT)
            && bounds_type2 == BOUNDS_TYPE::RECTANGULAR) {
            vec2 false_rotated_circle = {cos(-motion2.angle) * (-dp.x) - sin(-motion2.angle) * (-dp.y),
                sin(-motion2.angle) * (-dp.x) + cos(-motion2.angle) * (-dp.y)};
            vec2 first_quadrant_circle = abs(false_rotated_circle);
            if (first_quadrant_circle.x <= collider2.width/2.f + radius1/2.f
                && first_quadrant_circle.y <= collider2.height/2.f + radius1/2.f) {
                if (false_rotated_circle.x < collider2.width && false_rotated_circle.x > -collider2.width) {
                    if (false_rotated_circle.y > collider2.height/2.f
                        || false_rotated_circle.y < -collider2.height/2.f) {
                        return 2;
                    }
                }
                return 1;
            }

            // NOTE: maybe useful to find additional radial-rect collisions -- save for future debugging
            // if (sqrt(dot(first_quadrant_circle - vec2(collider2.width,collider2.height)/2.f, first_quadrant_circle - vec2(collider2.width,collider2.height)/2.f)) <= collider1.width/2.f)
            //     return 1;

            return 0;
        }

        std::array<vec2, 4> m1_bounding_points = get_bounding_points(collider1, e1);

        // One collider rectangular, one radial or point (opposite order)
        if ((bounds_type2 == BOUNDS_TYPE::RADIAL || bounds_type2 == BOUNDS_TYPE::POINT)
            && bounds_type1 == BOUNDS_TYPE::RECTANGULAR) {
            vec2 false_rotated_circle = {cos(-motion1.angle) * (dp.x) - sin(-motion1.angle) * (dp.y),
                sin(-motion1.angle) * (dp.x) + cos(-motion1.angle) * (dp.y)};
            vec2 first_quadrant_circle = abs(false_rotated_circle);
            if (first_quadrant_circle.x <= collider1.width/2.f + radius2/2.f
                && first_quadrant_circle.y <= collider1.height/2.f + radius2/2.f) {
                if (false_rotated_circle.x < collider1.width && false_rotated_circle.x > -collider1.width) {
                    if (false_rotated_circle.y > collider1.height/2.f
                        || false_rotated_circle.y < -collider1.height/2.f) {
                        return 2;
                    }
                }
                return 1;
            }

            // NOTE: maybe useful to find additional radial-rect collisions -- save for future debugging
            // if (sqrt(dot(first_quadrant_circle - vec2(collider1.width,collider1.height)/2.f, first_quadrant_circle - vec2(collider1.width,collider1.height)/2.f)) <= collider2.width/2.f)
                // return 1;

            return 0;
        }


        // Transform bounding points to screen positions
        for (vec2& point : m1_bounding_points) {
            point += motion1.position;
        }
        for (vec2& point : m2_bounding_points) {
            point += motion2.position;
        }

        // Both colliders rectangular
        // define all axis angles (normals to edges)
        float axis_angles[4];
        axis_angles[0] = motion1.angle;
        axis_angles[1] = M_PI_2 + motion1.angle;
        axis_angles[2] = motion2.angle;
        axis_angles[3] = M_PI_2 + motion2.angle;
        // see if an overlap exists in any of the axes
        for (float& angle : axis_angles) {
            if (no_overlap(m1_bounding_points, m2_bounding_points, angle)) {
                // LOG_INFO("No collision!");
                return 0;
            }
        }
        // LOG_INFO("Collision detected between motion with position ({}, {}) and "
        //          "motion with position ({}, {})",
        //          motion1.position.x, motion1.position.y, motion2.position.x,
        //          motion2.position.y);
        return 1;
    }
    return 0;
}
