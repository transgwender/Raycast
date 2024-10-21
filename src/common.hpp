#pragma once

// standard libs
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off
#include <gl3w.h>
#include <GLFW/glfw3.h>
// clang-format on

// The glm library provides vector and matrix operations as in GLSL
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/mat3x3.hpp>          // mat3
#include <glm/vec2.hpp>            // vec2
#include <glm/vec3.hpp>            // vec3
using namespace glm;

#include "ecs/ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() {
    return std::string(PROJECT_SOURCE_DIR) + "data";
};
inline std::string shader_path(const std::string& name) {
    return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;
};
inline std::string scene_path(const std::string& name) {
    return std::string(PROJECT_SOURCE_DIR) + "/scenes/" + name;
}
inline std::string textures_path(const std::string& name) {
    return data_path() + "/textures/" + std::string(name);
};
inline std::string music_path(const std::string& name) {
    return data_path() + "/audio/music/" + std::string(name);
};
inline std::string sfx_path(const std::string& name) {
    return data_path() + "/audio/sfx/" + std::string(name);
};
inline std::string sfx_dir_path() {
    return data_path() + "/audio/sfx/";
};
inline std::string mesh_path(const std::string& name) {
    return data_path() + "/meshes/" + std::string(name);
};

const int window_width_px = 1280;
const int window_height_px = 720;

static int native_width = 320;
static int native_height = 180;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 M_PI/2
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from
// ComponentNonCopyable
struct Transform {
    mat3 mat = {{1.f, 0.f, 0.f},
                {0.f, 1.f, 0.f},
                {0.f, 0.f, 1.f}}; // start with the identity
    void scale(vec2 scale);
    void rotate(float radians);
    void translate(vec2 offset);
};

bool checkGlErrors();
