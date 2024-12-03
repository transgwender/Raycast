// internal
#include "ai.hpp"

float walk_react_distance = 20.0f;
float walk_react_minisun_distance = 1000.0f;
float look_react_distance = 800.0f;

void AISystem::step(float elapsed_ms) {
    updateDash(elapsed_ms);
}

// Helper function to calculate squared distance between two positions
float AISystem::calculateDistanceSquared(const vec2& position1, const vec2& position2) {
    float dx = position1.x - position2.x;
    float dy = position1.y - position2.y;
    return dx * dx + dy * dy;
}

void AISystem::updateDash(float elapsed_ms) {
    for (const Entity& dashEntity : registry.turtles.entities) {
        Motion& dashMotion = registry.motions.get(dashEntity);
        DashTheTurtle& t = registry.turtles.get(dashEntity);
        float minimumDistanceSquared = std::numeric_limits<float>::max();
        vec2 minimumDisplacement = vec2(0, 0);
        bool foundMinisun = false;

        // Precompute squared comparison distances
        float walkReactMinisunDistanceSquared = walk_react_minisun_distance * walk_react_minisun_distance;
        float walkReactDistanceSquared = walk_react_distance * walk_react_distance;
        float lookReactDistanceSquared = look_react_distance * look_react_distance;

        // Check minisuns
        for (const Entity& sunEntity : registry.minisuns.entities) {
            if (registry.minisuns.get(sunEntity).lit) {
                Motion& sunMotion = registry.motions.get(sunEntity);
                float distanceSquared = calculateDistanceSquared(dashMotion.position, sunMotion.position);

                if (distanceSquared < minimumDistanceSquared) {
                    minimumDistanceSquared = distanceSquared;
                    minimumDisplacement = vec2(dashMotion.position.x - sunMotion.position.x,
                                               dashMotion.position.y - sunMotion.position.y);
                }
                foundMinisun = true;
            }
        }

        // Choose reaction based on distance squared
        float compareDistanceSquared = foundMinisun ? walkReactMinisunDistanceSquared : walkReactDistanceSquared;

        if (!t.tired) {
            if (minimumDistanceSquared <= compareDistanceSquared) {
                t.behavior = DASH_STATES::WALK;
                t.nearestLightRayDirection = minimumDisplacement;
                if (abs(minimumDisplacement.x) <= 20) {
                    t.behavior = DASH_STATES::STARE;
                }
            } else if (abs(dashMotion.position.x - t.originalPosition.x) > 1) {
                t.behavior = DASH_STATES::WALK;
                minimumDisplacement =
                    vec2(dashMotion.position.x - t.originalPosition.x, dashMotion.position.y - t.originalPosition.y);
                t.nearestLightRayDirection = minimumDisplacement;
            } else if (minimumDistanceSquared <= lookReactDistanceSquared) {
                t.behavior = DASH_STATES::STARE;
                t.nearestLightRayDirection = minimumDisplacement;
            } else {
                t.behavior = DASH_STATES::IDLE;
            }
        }
    }

    (void)elapsed_ms;
    (void)timeAccumulator;
}
