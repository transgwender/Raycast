#pragma once

// standard libs
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef __EMSCRIPTEN__
#include <GLFW/emscripten_glfw3.h>
#include <emscripten.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#else
#include <gl3w.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#define GLM_FORCE_PURE
#include <glm/mat3x3.hpp>          // mat3
#include <glm/vec2.hpp>            // vec2
#include <glm/vec3.hpp>            // vec3
using namespace glm;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ecs/ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:

inline std::string data_path() {
#ifdef __EMSCRIPTEN__
    return ""; // root directory in virtual file system
#else
    return "./data";
#endif
};
inline std::string shader_path(const std::string& name) {
    return data_path() + "/shaders/" + name;
};
inline std::string scene_path(const std::string& name) {
    return data_path() + "/scenes/" + name;
}
inline std::string textures_path(const std::string& name) {
    return data_path() + "/textures/" + std::string(name);
};
inline std::string mesh_path(const std::string& name) {
    return data_path() + "/meshes/" + std::string(name);
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
inline std::string font_path(const std::string& name) {
    return data_path() + "/fonts/" + name;
}
inline std::string background_path(const std::string& name) {
    return data_path() + "/backgrounds/" + name;
}
inline std::string player_data_path(const std::string& name) {
#ifdef __EMSCRIPTEN__
    return "/player_data/" + std::string(name);
#else
    return "./" + std::string(name);
#endif
}

extern int window_width_px;
extern int window_height_px;

extern int viewport_offset_x;
extern int viewport_offset_y;

extern int viewport_width;
extern int viewport_height;

extern int framebuffer_width;
extern int framebuffer_height;

extern int render_skips;

static int native_width = 320;
static int native_height = 180;

static int upscaled_width = 1280;
static int upscaled_height = 720;

static bool debug = false;

static int LIGHT_TIMER_MS = 2000/100;

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
