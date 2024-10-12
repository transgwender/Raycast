#pragma once
#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include <unordered_map>
#include <vector>

// Main data relevant to the level
struct Scene {
    std::string scene_tag;
};

// A level is also a scene, but it may store additional information.
struct Level {};

////////////////////////////////////////////////////////////////
///
/// Component Definitions
///
////////////////////////////////////////////////////////////////

// A zone represents the start and end points of light.
enum class ZONE_TYPE { START = 0, END = 1, ZONE_TYPE_COUNT };
struct Zone {
    vec2 position = {0, 0};
    ZONE_TYPE type;
};

// Entities that are `renderable` are visible in scenes.
struct Renderable {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    float angle = 0;
};

// Light source captures the characteristics of a source of light, such as the
// angle the light is shot at and the time interval between firing.
struct LightSource {
    float angle = 0;
};

struct Light {
    Entity last_reflected;
    float last_reflected_timeout;
};

// All data relevant to the shape and motion of entities
struct Motion {
    vec2 position = {0, 0};
    float angle = 0;
    vec2 velocity = {0, 0};
    vec2 scale = {10, 10};
    bool collides = true;
};

// Stucture to store collision information
struct Collision {
    // NOTE: The first object is stored in the ECS container.entities.
    Entity other; // The second object involved in the collision.
    explicit Collision(Entity& other) { this->other = other; };
};

// Object is reflective
struct Reflective {};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl &
// salmon.vs.glsl)
struct ColoredVertex {
    vec3 position;
    vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
    vec3 position;
    vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh {
    static bool loadFromOBJFile(std::string obj_path,
                                std::vector<ColoredVertex>& out_vertices,
                                std::vector<uint16_t>& out_vertex_indices,
                                vec2& out_size);
    vec2 original_size = {1, 1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
};

// Entities that are interactable are acted upon by user interaction (such as
// keyboard input, mouse clicks).
struct Interactable {};

// Represents a transition to another scene.
struct ChangeScene {
    std::string scene;
};

// Represents the bounding box for the entity it is applied to -- used to detect
// collisions.
struct BoundingBox {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
};

// Entities that are mounted on linear rails can move along a straight line.
struct OnLinearRails {
    float angle = 0;
    float length = 100; // This is really the "half" length.
    vec2 firstEndpoint = {0, 0};
    vec2 secondEndpoint = {0, 0};
    vec2 direction = {0, 0};
};

// Entities that are linearly interpolatable can make use of linear
// interpolation to perform smoothening of animations.
struct LinearlyInterpolatable {
    float t;
    bool should_switch_direction;
    float t_step = 0.01;
};

// Represents entities that can be rotated by the user
struct Rotateable {};

/**
 * The following enumerators represent global identifiers referring to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */
enum class TEXTURE_ASSET_ID {
    FISH,
    PLAY_BUTTON,
    START_ZONE,
    END_ZONE,
    LIGHT,
    TEXTURE_COUNT,
};
constexpr int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    TEXTURED,
    SCREEN,
    EFFECT_COUNT,
};
constexpr int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    SPRITE,
    SCREEN_TRIANGLE,
    GEOMETRY_COUNT,
};
constexpr int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
    TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
    GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};
