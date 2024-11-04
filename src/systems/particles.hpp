#pragma once
#include <random>

class ParticleSystem {
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;

public:
    void init();
    void step(float elapsed_ms);
};