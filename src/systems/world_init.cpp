#include "world_init.hpp"
#include "ecs/registry.hpp"

Entity createSprite(const Entity &entity, RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture) {
    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    auto& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = vec2({200, 200});
    if (registry.reflectives.has(entity)) {
        motion.scale = vec2({20, 200});
        motion.angle = 120 * 180 / M_PI;

    }
    if (registry.zones.has(entity)) {
        motion.scale = vec2({128, 128});
        motion.angle = M_PI / 4;
    }

    registry.renderRequests.insert(entity, {texture,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity createLight(const Entity &entity, RenderSystem* renderer, vec2 position, vec2 velocity) {
    Entity light =
        createSprite(entity, renderer, position, TEXTURE_ASSET_ID::LIGHT);

    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.lightRays.emplace(light);

    auto& motion = registry.motions.get(light);
    motion.velocity = velocity;
    motion.scale = vec2({20, 80});
    motion.angle = M_PI_2 + atan2(velocity.y, velocity.x);
    motion.collides = true;

    return entity;
}

void setZone(Entity entity, ZONE_TYPE zType, vec2 position) {
    Zone zone = registry.zones.emplace(entity);
    zone.position = position;
    zone.type = zType;
}
