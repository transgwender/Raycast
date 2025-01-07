#pragma once

#include "common.hpp"

inline mat3 createProjectionMatrix() {
    constexpr float left = 0.f;
    constexpr float top = 0.f;

    const auto right = static_cast<float>(native_width);
    const auto bottom = static_cast<float>(native_height);

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

inline vec2 screenToWorld(const vec2 screenPos) {
    const vec2 window_scale = vec2(framebuffer_width / window_width_px, framebuffer_height / window_height_px);
    const vec2 scaled_screen_pos = screenPos * window_scale;
    const vec2 offset_pos = scaled_screen_pos - vec2(viewport_offset_x, viewport_offset_y);
    const vec2 scaled_offset_pos = offset_pos / vec2(viewport_width, viewport_height);
    const vec2 world_pos = vec2(native_width, native_height) * scaled_offset_pos;
    return world_pos;
}
