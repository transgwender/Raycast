#pragma once

#include "common.hpp"
#include "ecs/ecs.hpp"
#include "render.hpp"

const size_t LIGHT_SPAWN_DELAY_MS = 2000 * 3;

Entity createSprite(const Entity &entity, RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);
Entity createLight(const Entity& entity, RenderSystem* renderer, vec2 position,
                   vec2 velocity);
void setZone(Entity entity, ZONE_TYPE zType, vec2 position);