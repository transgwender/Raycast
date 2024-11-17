#pragma once

#include "ai.hpp"
#include "common.hpp"
#include "ecs/ecs.hpp"
#include "math.hpp"
#include "render/render.hpp"

Entity createSprite(const Entity &entity, vec2 position, vec2 scale, float angle, const std::string& textureName, Layer layer = FOREGROUND, vec4 color = vec4(255, 255, 255, 255));
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
Entity createPortalEntity(vec2 position, float angle, const vec2& size, const std::string& sprite_name);
void createPortals(vec2 pos1, float angle1, vec2 pos2, float angle2);
void linkPortals(const Entity& portal_1, const Entity& portal_2, const vec2& pos1, float angle1, const vec2& pos2, float angle2);
void initMesh(const Entity& entity, const std::string& mesh_name, const vec2& position, float angle, const vec2& scale);

void setZone(const Entity& entity, ZONE_TYPE zType, vec2 position);
void initLinearRails(const Entity& entity, OnLinearRails);