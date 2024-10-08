#pragma once

#include "../common.hpp"
#include "ecs/ecs.hpp"
#include "render.hpp"

Entity createSprite(const Entity &entity, RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);
void setZone(Entity entity, ZONE_TYPE zType, vec2 position);