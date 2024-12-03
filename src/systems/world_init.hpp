#pragma once

#include "ai.hpp"
#include "common.hpp"
#include "ecs/ecs.hpp"
#include "math.hpp"
#include "render/render.hpp"

class world_init {
  public:
    static unsigned int portal_color_i;
    vec4 portal_colors[8] = {
        vec4(255, 223, 0, 255),     // Yellow
        vec4(255, 255, 255, 255),   // White
        vec4(255, 105, 180, 255),   // Hot Pink
        vec4(0, 255, 255, 255),     // Cyan
        vec4(255, 30, 30, 255),     // Red
        vec4(148, 0, 211, 255),     // Dark Violet
        vec4(100, 248, 100, 255),   // Light Green
        vec4(100, 150, 230, 255),   // Light Blue
    };
};

Entity createSprite(const Entity& entity, vec2 position, vec2 scale, float angle, const std::string& textureName,
                    Layer layer = FOREGROUND, vec4 color = vec4(255, 255, 255, 255));
Entity createMirror(const Entity& entity, const Mirror& mirror);

Entity createLight(const Entity& entity, vec2 position, float dir);
Entity createDashTheTurtle(const Entity& entity, vec2 position);
Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label);
Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                         const std::string& textureName, vec3 color = {255, 255, 255});
Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                               const std::string& textureName, const std::string& nextScene,
                               vec3 color = {255, 255, 255});
Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                          const std::string& textureName);
Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                               const std::string& nextScene);
Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth,
                         float cellHeight, const std::vector<unsigned int>& animationFrames);
Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth,
                         float cellHeight, const std::vector<unsigned int>& animationFrames,
                         const std::string& textureName = "turtle_sprite_sheet", float imageWidth = 0,
                         float imageHeight = 0, const vec4& color = {255, 255, 255, 255}, float angle = 0);
Entity createLever(const Entity& affectedEntity, const vec2& position, LEVER_STATES state, LEVER_EFFECTS effect,
                   LEVER_STATES activeLever);
Entity createPortalEntity(vec2 position, float angle, const vec2& size, const vec4& color);
void createPortals(vec2 pos1, float angle1, vec2 pos2, float angle2);
void linkPortals(const Entity& portal_1, const Entity& portal_2, const vec2& pos1, float angle1, const vec2& pos2,
                 float angle2, const vec4& color);
void initMesh(const Entity& entity, const std::string& mesh_name, const vec2& position, float angle, const vec2& scale);

void setZone(const Entity& entity, ZONE_TYPE zType, vec2 position);
void initLinearRails(const Entity& entity, OnLinearRails);