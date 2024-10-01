#pragma once

#include "../common.hpp"
#include "ecs/ecs.hpp"
#include "render.hpp"

Entity createSprite(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);
