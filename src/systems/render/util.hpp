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
    vec2 worldPos = vec2(screenPos.x - viewport_offset_x / 2, screenPos.y - viewport_offset_y / 2);
    worldPos.x = (worldPos.x / (window_width_px - viewport_offset_x)) * native_width;
    worldPos.y = (worldPos.y / (window_height_px - viewport_offset_y)) * native_height;
    return worldPos;
}
