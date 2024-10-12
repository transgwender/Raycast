#pragma once

#include "../common.hpp"
#include "ecs/ecs.hpp"
#include "render.hpp"

Entity createSprite(const Entity &entity, RenderSystem* renderer, vec2 position, vec2 scale, float angle, TEXTURE_ASSET_ID texture);
Entity createLight(const Entity& entity, RenderSystem* renderer, vec2 position,
                   vec2 velocity);
Entity createMirror(const Entity& entity, RenderSystem* renderer, vec2 position, float angle);
void setZone(Entity entity, ZONE_TYPE zType, vec2 position);