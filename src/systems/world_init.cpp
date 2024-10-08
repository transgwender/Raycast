#include "world_init.hpp"
#include "ecs/registry.hpp"

Entity createSprite(const Entity &entity, RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture) {

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    auto& motion = registry.motions.emplace(entity);
    motion.position = position;

    motion.scale = vec2({200, 200});

    registry.renderRequests.insert(entity, {texture,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}