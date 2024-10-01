#include "world_init.hpp"
#include "ecs/registry.hpp"

Entity createFish(RenderSystem* renderer, vec2 position) {
    // Reserve an entity
    const auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.position = position;

    // Setting initial values, scale is negative to make it face the opposite way
    motion.scale = vec2({200, 200});

    // Create an (empty) Bug component to be able to refer to all bug
    registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::FISH,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}
