// internal
#include "ai.hpp"

float walk_react_distance = 20.0f;
float walk_react_minisun_distance = 1000.0f;
float look_react_distance = 800.0f;

void AISystem::step(float elapsed_ms) { 

    updateDash();

    (void)elapsed_ms; // placeholder to silence unused warning until implemented
}


void AISystem::updateDash() {
    for (Entity dashEntity : registry.turtles.entities) {
        Motion& dashMotion = registry.motions.get(dashEntity);
        float minimumDistance = std::numeric_limits<float>::max();
        //Entity minimumLightEntity;
        vec2 minimumDisplacement = vec2(0, 0);
        bool foundMinisun = false;


        for (Entity sunEntity : registry.minisuns.entities) {
            if (registry.minisuns.get(sunEntity).lit) {
                Motion& sunMotion = registry.motions.get(sunEntity);
                float dx = dashMotion.position.x - sunMotion.position.x;
                float dy = dashMotion.position.y - sunMotion.position.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < minimumDistance) {
                    minimumDistance = distance;
                    // minimumLightEntity = lightEntity;
                    minimumDisplacement = vec2(dx, dy);
                }
                foundMinisun = true;
            }
        }

        if (!foundMinisun) {
            for (Entity lightEntity : registry.lightRays.entities) {
                Motion& lightMotion = registry.motions.get(lightEntity);
                float dx = dashMotion.position.x - lightMotion.position.x;
                float dy = dashMotion.position.y - lightMotion.position.y;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < minimumDistance) {
                    minimumDistance = distance;
                    // minimumLightEntity = lightEntity;
                    minimumDisplacement = vec2(dx, dy);
                }
            }
        }

        float compareDistance = foundMinisun ? walk_react_minisun_distance : walk_react_distance;
        if (minimumDistance <= compareDistance) {
            registry.turtles.get(dashEntity).behavior = DASH_STATES::WALK;
            registry.turtles.get(dashEntity).nearestLightRayDirection = minimumDisplacement;
            //registry.turtles.get(dashEntity).closestLightRay = minimumLightEntity;
        } else if (minimumDistance <= look_react_distance) {
            registry.turtles.get(dashEntity).behavior = DASH_STATES::STARE;
            registry.turtles.get(dashEntity).nearestLightRayDirection = minimumDisplacement;
            //registry.turtles.get(dashEntity).closestLightRay = minimumLightEntity;
        } else {
            registry.turtles.get(dashEntity).behavior = DASH_STATES::IDLE;
            // registry.turtles.get(dashEntity).closestLightRay = minimumLightEntity;
        }

        // printf("%lu\n", registry.lightRays.entities.size());

        if (registry.lightRays.entities.size() == 0) {
            registry.turtles.get(dashEntity).behavior = DASH_STATES::IDLE;
        }
    }
}
