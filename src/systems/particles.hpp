#pragma once
#include "common.hpp"
#include "components.hpp"
#include <random>

class ParticleSystem {
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;

public:
    void init();
    void step(float elapsed_ms);

    static Entity createLightDissipation(const Motion& light_motion);
    static Entity createPortalParticles(const Portal& portal, const vec4& color);
};