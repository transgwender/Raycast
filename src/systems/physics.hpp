#pragma once

#include "common.hpp"
#include "components.hpp"
#include "ecs/ecs.hpp"
#include "ecs/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  public:
    void step(float elapsed_ms);

    PhysicsSystem() = default;
};

bool approximatelyEqual(float a, float b, float epsilon);
bool essentiallyEqual(float a, float b, float epsilon);
bool definitelyGreaterThan(float a, float b, float epsilon);
bool definitelyLessThan(float a, float b, float epsilon);
bool approximatelyEqual(float a, float b);
bool essentiallyEqual(float a, float b);
bool definitelyGreaterThan(float a, float b);
bool definitelyLessThan(float a, float b);
