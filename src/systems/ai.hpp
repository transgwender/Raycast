#pragma once

#include <vector>

#include "common.hpp"
#include "registry.hpp"

class AISystem {
  public:
    void step(float elapsed_ms);
    void updateDash();
};
