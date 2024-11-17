#include "common.hpp"
#include <iostream>

int window_width_px = 1280;
int window_height_px = 720;

// Note, we could also use the functions from GLM but we write the
// transformations here to show the uderlying math
void Transform::scale(vec2 scale) {
    mat3 S = {{scale.x, 0.f, 0.f}, {0.f, scale.y, 0.f}, {0.f, 0.f, 1.f}};
    mat = mat * S;
}

void Transform::rotate(float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat3 R = {{c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}};
    mat = mat * R;
}

void Transform::translate(vec2 offset) {
    mat3 T = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {offset.x, offset.y, 1.f}};
    mat = mat * T;
}

/**
 * Check GL error state for any errors.
 * Returns `false` if there were no errors.
 * Otherwise, prints
 */
bool checkGlErrors() {
    GLenum error = glGetError();

    bool encounteredError = false;

    while (error != GL_NO_ERROR) {
        encounteredError = true;
        auto error_str = "";
        switch (error) {
        case GL_INVALID_OPERATION:
            error_str = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_str = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:;
        }
        LOG_ERROR("[OpenGL]: {}", error_str);
        error = glGetError();
    }

    return encounteredError;
}