#pragma once
#include "components.hpp"

class AnimationSystem {
    float animation_speed = 160.f;

    void animateLever(const Entity& entity, SpriteSheet& ss) const;

public:
    void step(float elapsed_ms) const;

};