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

// If your mouse or screenPos is in [0..w, 0..h] from top-left:
inline vec2 screenToWorld(const vec2 screenPos) {
    // First, shift the coordinates so that (0,0) is the top-left of the
    // *game viewport* (instead of the top-left of the entire window).
    float xInViewport = screenPos.x - viewport_offset_x;
    float yInViewport = screenPos.y - viewport_offset_y;

    // Next, normalize these coordinates to [0..1] by dividing by
    // the actual viewport's width and height.
    float nx = xInViewport / viewport_width;
    float ny = yInViewport / viewport_height;

    // Finally, scale them up to your native resolution.
    // If your in-game coordinate system has (0,0) at the top-left and
    // Y increases down, then you can keep ny as-is.
    float worldX = nx * native_width;
    float worldY = ny * native_height;

    return {worldX, worldY};

    //vec2 worldPos = vec2(screenPos.x - viewport_offset_x / 2, screenPos.y - viewport_offset_y / 2);
    //worldPos.x = (worldPos.x / (window_width_px - viewport_offset_x)) * native_width;
    //worldPos.y = (worldPos.y / (window_height_px - viewport_offset_y)) * native_height;
    //return worldPos;
}


