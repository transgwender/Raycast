#pragma once
#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include <unordered_map>
#include <vector>

// Scenes
enum class SCENE_ASSET_ID {
    TEST = 0,
    MAIN_MENU = TEST + 1,
    LEVEL1 = MAIN_MENU + 1,
    MIRRORS_TEST = LEVEL1 + 1,
    SCENE_COUNT = MIRRORS_TEST + 1 
};
constexpr int scene_count = (int)SCENE_ASSET_ID::SCENE_COUNT;

// Main data relevant to the level
struct Scene {
    SCENE_ASSET_ID scene;
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
    SCENE_ASSET_ID scene;
};

struct BoundingBox {
    vec2 position = {0, 0};
    vec2 scale = {10, 10};
};

struct OnLinearRails {
  float angle = 0;
  float length = 100;
  vec2 firstEndpoint = {0,0};
  vec2 secondEndpoint = {0,0};
  vec2 direction = {0,0};
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
    PLAY_BUTTON = FISH + 1,
    START_ZONE = PLAY_BUTTON + 1,
    END_ZONE = START_ZONE + 1,
    LIGHT = END_ZONE + 1,
    TEXTURE_COUNT = LIGHT + 1,
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
