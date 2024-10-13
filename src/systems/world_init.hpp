#pragma once

#include "common.hpp"
#include "ecs/ecs.hpp"
#include "render/render.hpp"

Entity createSprite(const Entity &entity, vec2 position, vec2 scale, float angle, const std::string& textureName, const std::string& shaderName = "textured");
Entity createMirror(const Entity& entity, vec2 position, float angle);

Entity createLight(const Entity& entity, vec2 position, vec2 velocity);

void setZone(Entity entity, ZONE_TYPE zType, vec2 position);