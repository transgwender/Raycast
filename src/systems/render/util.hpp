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
    int scaled_offset_x = viewport_offset_x / window_scale_x;
    int scaled_offset_y = viewport_offset_y / window_scale_y;
    vec2 worldPos = vec2(screenPos.x - scaled_offset_x, screenPos.y - scaled_offset_y);
    worldPos.x = (worldPos.x / (window_width_px - (scaled_offset_x * window_scale_x))) * native_width;
    worldPos.y = (worldPos.y / (window_height_px - (scaled_offset_y * window_scale_y))) * native_height;
    return worldPos;
}
