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
    static const float MaxOrbitDistance;
    static const float MaxOrbitAngle;
    static const float MaxAngleToTravel;

    void step(float elapsed_ms);
    void update_positions(float elapsed_ms);
    void exert_blackhole_pull(float elapsed_ms);
    void updateVelocityFromBlackholePull(Entity& light_ray_entity, Entity& blackhole_entity, Motion& blackhole_motion,
                                         Motion& light_ray_motion, Blackhole& blackhole, int& retFlag);
    void updateVelocityDuringOrbit(Entity& light_ray_entity, Motion& light_ray_motion, Motion& blackhole_motion,
                                   const float t);
    void startLightOrbit(Entity& light_ray_entity, Motion& light_ray_motion, Motion& blackhole_motion,
                         Entity& blackhole_entity);
    void detect_collisions();
    bool should_light_orbit(Entity light, Entity blackhole);
    PhysicsSystem() = default;
  private:

    static bool shouldStep();
};

