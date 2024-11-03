#pragma once

#include <vector>

#include "common.hpp"
#include "registry.hpp"

class AISystem {
  public:
    void step(float elapsed_ms);
    void updateDash(float elapsed_ms);
private:
    float timeAccumulator = 0.0f;
    static float calculateDistanceSquared(const vec2& position1, const vec2& position2);
};
