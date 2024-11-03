#pragma once

#include "common.hpp"
#include "components.hpp"
#include "ecs/ecs.hpp"
#include "ecs/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  public:
    /// Gravitational constant for our 2D light-maze world
    static const float GravitationalConstant;
    /// Speed of light for our 2D light-maze world
    static const float SpeedOfLight;

    void step(float elapsed_ms);
    PhysicsSystem() = default;
  private:

    bool shouldStep();
};

