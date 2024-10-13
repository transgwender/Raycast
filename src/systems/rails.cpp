#include "registry.hpp"
#include "utils/math.hpp"

namespace raycast {
  namespace rails {
    const float ONE_SECOND = 1000.f;
    void init() {
      // Calculate the endpoints for all entities on rails
      auto& entities_on_linear_rails = registry.entitiesOnLinearRails.entities;
      LOG_INFO("Calculating endpoints for the {} entities on rails.",
          entities_on_linear_rails.size());
      for (auto e : entities_on_linear_rails) {
        Motion& e_motion = registry.motions.get(e);
        OnLinearRails& e_rails = registry.entitiesOnLinearRails.get(e);
        LinearlyInterpolatable& e_lr = registry.linearlyInterpolatables.get(e);

        auto direction = vec2(cos(e_rails.angle), sin(e_rails.angle));
        vec2 firstEndpoint = e_motion.position + e_rails.length * direction;
        vec2 secondEndpoint = e_motion.position - e_rails.length * direction;

        e_lr.t = 0.5;
        e_rails.firstEndpoint = firstEndpoint;
        e_rails.secondEndpoint = secondEndpoint;
        e_rails.direction = direction;
        LOG_INFO("Entity with position: ({}, {}) on a linear rail has "
            "endpoints: ({},{}) ({}, {}) with a direction: ({},{})",
            e_motion.position.x, e_motion.position.y, firstEndpoint.x,
            firstEndpoint.y, secondEndpoint.x, secondEndpoint.y,
            direction.x, direction.y);

      }
    }

    void step(float elapsed_ms) {
      // Step all entities on rails
      auto& linear_rails_registry = registry.entitiesOnLinearRails;
      for (uint i = 0; i < linear_rails_registry.size(); i++) {
        auto e = linear_rails_registry.entities[i];
        OnLinearRails& r = linear_rails_registry.components[i];
        Motion& m = registry.motions.get(e);
        LinearlyInterpolatable& lr = registry.linearlyInterpolatables.get(e);
        float t = elapsed_ms / ONE_SECOND;
        if (lr.should_switch_direction) {
          lr.t += t * lr.t_step;
        } else {
          lr.t -= t * lr.t_step;
        }
        lr.t = raycast::math::clamp(0.0, 1.0, lr.t);
        m.position =
          raycast::math::lerp(r.firstEndpoint, r.secondEndpoint, lr.t);
      }
    }
  }
}
