#pragma once

#include "ai.hpp"
#include "common.hpp"
#include "ecs/ecs.hpp"
#include "math.hpp"
#include "render/render.hpp"

Entity createSprite(const Entity &entity, vec2 position, vec2 scale, float angle, const std::string& textureName, const std::string& shaderName = "textured");
Entity createMirror(const Entity& entity, const Mirror& mirror);

Entity createLight(const Entity& entity, vec2 position, float dir);
Entity createDashTheTurtle(const Entity& entity, vec2 position);
Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label);
Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName, vec3 color = {255, 255, 255});
Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName, const std::string& nextScene, vec3 color = {255, 255, 255});
Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName);
Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& nextScene);
Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth, float cellHeight, const std::vector<unsigned int>& animationFrames);
Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth,
                         float cellHeight, const std::vector<unsigned int>& animationFrames,
                         const std::string& textureName = "turtle_sprite_sheet", float imageWidth = 0, float imageHeight = 0);

Entity createLever(const Entity& affectedEntity, const vec2& position, LEVER_STATES state, LEVER_EFFECTS effect,
                   LEVER_STATES activeLever);
void initMesh(const Entity& entity, const std::string& mesh_name, const vec2& position, const float angle, const vec2& scale);

void setZone(const Entity& entity, ZONE_TYPE zType, vec2 position);
void initLinearRails(const Entity& entity, OnLinearRails);