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
#ifdef __EMSCRIPTEN__
    vec2 worldPos = vec2(screenPos.x, screenPos.y);
    worldPos.x = (worldPos.x / window_width_px) * native_width;
    worldPos.y = (worldPos.y / window_height_px) * native_height;
    return worldPos;
#endif

    const vec2 window_scale = vec2(static_cast<float>(framebuffer_width) / static_cast<float>(window_width_px), static_cast<float>(framebuffer_height) / static_cast<float>(window_height_px));
    const vec2 scaled_screen_pos = screenPos * window_scale;
    const vec2 offset_pos = scaled_screen_pos - vec2(viewport_offset_x, viewport_offset_y);
    const vec2 scaled_offset_pos = offset_pos / vec2(viewport_width, viewport_height);
    const vec2 world_pos = vec2(native_width, native_height) * scaled_offset_pos;

    // LOG_INFO("\n framebuffer dimensions: {}, {}\n window scale: {}, {}\n framebuffer position: {}, {}\n viewport "
    //          "dimensions: {}, {}\n viewport offset: {}, {}\n offset screen pos: {}, {}\n world pos: {}, {}\n",
    //          framebuffer_width, framebuffer_height, window_scale.x, window_scale.y, scaled_screen_pos.x,
    //          scaled_screen_pos.y, viewport_width, viewport_height, viewport_offset_x, viewport_offset_y, offset_pos.x,
    //          offset_pos.y, world_pos.x, world_pos.y);

    return world_pos;
}
