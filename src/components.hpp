#pragma once
#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include <unordered_map>
#include <vector>

// Main data relevant to the level
struct Scene {
    std::string scene_tag;
};

struct Level {

};

enum class ZONE_TYPE {
    START = 0,
    END = 1,
    ZONE_TYPE_COUNT
};

struct Zone {
    vec2 position = {0, 0};
    ZONE_TYPE type;
};


struct Renderable {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
    float angle = 0;
};

struct LightSource {
    float angle = 0;
};

struct Light {

};

// All data relevant to the shape and motion of entities
struct Motion {
    vec2 position = {0, 0};
    float angle = 0;
    vec2 velocity = {0, 0};
    vec2 scale = {10, 10};
};

// Stucture to store collision information
struct Collision {
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
    Collision(Entity& other) { this->other = other; };
};

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

struct Interactable {
};

struct ChangeScene {
    std::string scene;
};

struct BoundingBox {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
};

/**
 * The following enumerators represent global identifiers referring to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
    FISH = 0,
    PLAY_BUTTON = 1,
    START_ZONE = 2,
    END_ZONE = 3,
    LIGHT = 4,
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