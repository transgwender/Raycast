#include "collisions.h"

#include "ai.hpp"
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
// Returns 0 if no overlap
// vec2.x:
// Returns 1 if collision along sprite y side of rectangle
// Returns 2 if collision along sprite x side of rectangle
// vec2.y
// Returns magnitude of overlap;
vec2 Collisions::overlap(const Entity& e1, const Entity& e2, bool user_interaction) {
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
                return {1, max_possible_collision_distance/2.f - dist_squared};
            }
            return {0, 0.f};
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
                // calculate magnitude of overlap, transform to screen coordinates and
                // move radial object back that amount along its velocity vector
                vec2 overlap = {collider2.width/2.f + radius1/2.f - first_quadrant_circle.x,
                    collider2.height/2.f + radius1/2.f - first_quadrant_circle.y};
                // LOG_INFO("Overlap: ({}, {})\n", overlap.x, overlap.y);
                float overlap_magnitude = min(overlap.x, overlap.y);
                // LOG_INFO("Overlap: ({}, {}), magnitude: {}\n", overlap.x, overlap.y, overlap_magnitude);
                if (false_rotated_circle.x < collider2.width && false_rotated_circle.x > -collider2.width) {
                    if (false_rotated_circle.y > collider2.height/2.f
                        || false_rotated_circle.y < -collider2.height/2.f) {
                        return {2, overlap_magnitude};
                    }
                }
                return {1, overlap_magnitude};
            }

            // NOTE: maybe useful to find additional radial-rect collisions -- save for future debugging
            // if (sqrt(dot(first_quadrant_circle - vec2(collider2.width,collider2.height)/2.f, first_quadrant_circle - vec2(collider2.width,collider2.height)/2.f)) <= collider1.width/2.f)
            //     return 1;

            return {0, 0.f};
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
                // calculate magnitude of overlap, transform to screen coordinates
                // and move radial object back that amount along its velocity vector
                vec2 overlap = {collider1.width/2.f + radius2/2.f - first_quadrant_circle.x,
                    collider1.height/2.f + radius2/2.f - first_quadrant_circle.y};
                float overlap_magnitude = min(overlap.x, overlap.y) * 1.01;
                // LOG_INFO("Overlap: ({}, {}), magnitude: {}\n", overlap.x, overlap.y, overlap_magnitude);
                if (false_rotated_circle.x < collider1.width && false_rotated_circle.x > -collider1.width) {
                    if (false_rotated_circle.y > collider1.height/2.f
                        || false_rotated_circle.y < -collider1.height/2.f) {
                        return {2, overlap_magnitude};
                    }
                }
                return {1, overlap_magnitude};
            }

            // NOTE: maybe useful to find additional radial-rect collisions -- save for future debugging
            // if (sqrt(dot(first_quadrant_circle - vec2(collider1.width,collider1.height)/2.f, first_quadrant_circle - vec2(collider1.width,collider1.height)/2.f)) <= collider2.width/2.f)
                // return 1;

            return {0, 0.f};
        }

        // Transform bounding points to screen positions
        for (vec2& point : m1_bounding_points) {
            point += motion1.position;
        }
        for (vec2& point : m2_bounding_points) {
            point += motion2.position;
        }

        // mesh-rectangle collisions
        if (collider2.bounds_type == BOUNDS_TYPE::MESH
            && (collider1.bounds_type == BOUNDS_TYPE::RECTANGULAR || collider1.bounds_type == BOUNDS_TYPE::RADIAL) ||
            collider1.bounds_type == BOUNDS_TYPE::MESH
            && (collider2.bounds_type == BOUNDS_TYPE::RECTANGULAR || collider2.bounds_type == BOUNDS_TYPE::RADIAL)) {
            // LOG_INFO("Mesh collision possible — rectangular-mesh, Positions: ({}, {}), ({}, {})",
                // motion1.position.x, motion1.position.y, motion2.position.x, motion2.position.y);
            float axis_angles[5];
            Mesh& mesh = collider2.bounds_type == BOUNDS_TYPE::MESH ? registry.meshes.get(e2) : registry.meshes.get(e1);
            std::array<vec2, 4> rect_bounding_points = collider2.bounds_type == BOUNDS_TYPE::MESH ? m1_bounding_points : m2_bounding_points;
            Motion& mesh_motion = collider2.bounds_type == BOUNDS_TYPE::MESH ? motion2 : motion1;
            Motion& rect_motion = collider2.bounds_type == BOUNDS_TYPE::MESH ? motion1 : motion2;
            // rectangular mesh axes
            axis_angles[0] = rect_motion.angle;
            axis_angles[1] = M_PI_2 + rect_motion.angle;

            bool separated = true;

            uint NUM_VERTICES = mesh.vertex_indices.size();
            // LOG_INFO("Num vertices: {}", NUM_VERTICES);
            for (uint i = 0; i < NUM_VERTICES; i += 3)
            {
                std::vector<ColoredVertex> curr_face = {
                    mesh.vertices[mesh.vertex_indices[i + 0]],
                    mesh.vertices[mesh.vertex_indices[i + 1]],
                    mesh.vertices[mesh.vertex_indices[i + 2]]};
                Transform transform;

                transform.rotate(mesh_motion.angle);
                for (ColoredVertex &v : curr_face)
                {
                    v.position = vec3(
                            (v.position.x * mesh_motion.scale.x),
                            (v.position.y * mesh_motion.scale.y),
                            1.f);
                    v.position = transform.mat * v.position;
                    v.position += vec3{mesh_motion.position.x, mesh_motion.position.y, 0};
                }
                std::vector<vec2> face_points = {
                    vec2(curr_face[0].position.x, curr_face[0].position.y),
                    vec2(curr_face[1].position.x, curr_face[1].position.y),
                    vec2(curr_face[2].position.x, curr_face[2].position.y)};

                // calculate the axis angle for each edge of the triangular face
                axis_angles[2] = atan2(face_points[1].x - face_points[0].x, face_points[0].y - face_points[1].y);
                axis_angles[3] = atan2(face_points[2].x - face_points[0].x, face_points[0].y - face_points[2].y);
                axis_angles[4] = atan2(face_points[2].x - face_points[1].x, face_points[1].y - face_points[2].y);
                separated = false;
                for (float& angle : axis_angles) {
                    separated |= no_overlap(rect_bounding_points, {face_points[0],
                        face_points[1], face_points[2], face_points[2]}, angle);
                }
                if (!separated) {
                    return {1, 0.f};
                }
            }
            return {0, 0.f};
        }

        //
        // // second collider is a mesh, first collider is radial
        // if (collider2.bounds_type == BOUNDS_TYPE::MESH
        //     && collider1.bounds_type == BOUNDS_TYPE::RADIAL || collider1.bounds_type == BOUNDS_TYPE::POINT) {
        //     Mesh& mesh2 = registry.meshes.get(e2);
        //
        //     uint NUM_VERTICES = mesh2.vertex_indices.size();
        //     for (uint i = 0; i < NUM_VERTICES; i += 3)
        //     {
        //         std::vector<ColoredVertex> curr_face = {
        //             mesh2.vertices[mesh2.vertex_indices[i + 0]],
        //             mesh2.vertices[mesh2.vertex_indices[i + 1]],
        //             mesh2.vertices[mesh2.vertex_indices[i + 2]]};
        //         Transform transform;
        //         transform.rotate(motion2.angle);
        //         for (ColoredVertex &v : curr_face)
        //         {
        //             v.position = vec3(
        //                     motion2.position.x + (v.position.x * motion2.scale.x),
        //                     motion2.position.y + (v.position.y * motion2.scale.y),
        //                     1.f);
        //             v.position = transform.mat * v.position;
        //         }
        //         std::vector<vec2> face_points = {
        //             vec2(curr_face[0].position.x, curr_face[0].position.y),
        //             vec2(curr_face[1].position.x, curr_face[1].position.y),
        //             vec2(curr_face[2].position.x, curr_face[2].position.y)};
        //
        //         // Calculate area of the face
        //         vec2 v0 = face_points[0];
        //         vec2 v1 = face_points[1] - v0;
        //         vec2 v2 = face_points[2] - v0;
        //         // calculate area (note: calculated area is twice actual area)
        //         float total_area = sqrt(dot(v1,v1) * dot(v2,v2)
        //                 - (dot(v1,v2) * dot(v1,v2)));
        //
        //         // mesh 1 inside mesh 2
        //         vec2 point = motion1.position;
        //         // calculate sum of areas of triangles created from point to
        //         // two of the current mesh triangle areas
        //         float sub_area = 0.f;
        //         // v0->v1->point triangle
        //         sub_area += sqrt(dot(point - v0,point - v0) * dot(v1 - v0,v1 - v0)
        //         - (dot(point - v0, v1 - v0) * (dot(point - v0, v1 - v0))));
        //         // v0->v2->point triangle
        //         sub_area += sqrt(dot(point - v0,point - v0) * dot(v2 - v0,v2 - v0)
        //         - (dot(point - v0, v2 - v0) * (dot(point - v0, v2 - v0))));
        //         // v2->v2->point triangle
        //         sub_area += sqrt(dot(v1 - point, v1 - point) * dot(v1 - v2,v1 - v2)
        //         - (dot(v1 - point, v1 - v2) * (dot(v1 - point, v1 - v2))));
        //         if (sub_area <= total_area) {
        //             return 1;
        //         }
        //     }
        //     return 0;
        // }


        // Else assume both colliders rectangular
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
                return {0, 0.f};
            }
        }
        return {1, 0.f};
    }
    return {0, 0.f};
}