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