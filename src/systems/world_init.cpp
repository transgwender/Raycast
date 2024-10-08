#include "world_init.hpp"
#include "ecs/registry.hpp"

Entity createSprite(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture) {
    const auto entity = Entity();

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

void setZone(Entity entity, ZONE_TYPE zType, vec2 position) {
    Zone zone = registry.zones.emplace(entity);
    zone.position = position;
    zone.type = zType;
}
