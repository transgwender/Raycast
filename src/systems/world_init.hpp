#pragma once

#include "common.hpp"
#include "ecs/ecs.hpp"
#include "render/render.hpp"

Entity createSprite(const Entity &entity, vec2 position, vec2 scale, float angle, const std::string& textureName, const std::string& shaderName = "textured");
Entity createMirror(const Entity& entity, vec2 position, float angle);

Entity createLight(const Entity& entity, vec2 position, vec2 velocity);
Entity createDashTheTurtle(const Entity& entity, vec2 position);

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label);
Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName);
Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName, const std::string& nextScene);
Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName);

void setZone(Entity entity, ZONE_TYPE zType, vec2 position);