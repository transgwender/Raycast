#include "world_init.hpp"
#include "ecs/registry.hpp"

Entity createSprite(const Entity& entity, const vec2 position, const vec2 scale, float angle,
                    const std::string& textureName,
                    const std::string& shaderName) {
    auto& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.angle = angle;

    registry.materials.insert(entity, {textureName, shaderName});

    return entity;
}

Entity createLight(const Entity &entity, vec2 position, vec2 velocity) {
    vec2 scale = vec2({8, 8});
    float angle = M_PI_2 + atan2(velocity.y, velocity.x);

    Entity light = createSprite(entity, position, scale, angle, "light");

    registry.lightRays.emplace(light);

    auto& motion = registry.motions.get(light);
    motion.velocity = velocity;
    motion.collides = true;

    PointLight& point_light = registry.pointLights.emplace(light);
    point_light.diffuse = 6.0f * vec3(255, 233, 87);
    point_light.linear = 0.045f;
    point_light.quadratic = 0.0075;

    return entity;
}

Entity createMirror(const Entity& entity, vec2 position, float angle) {
    vec2 scale = vec2({5, 40});
    createSprite(entity, position, scale, angle, "mirror");
    registry.reflectives.emplace(entity);
    return entity;
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label) {
    createSprite(entity, position, scale, 0, "button");
    registry.interactables.emplace(entity);
    BoundingBox boundingBox;
    boundingBox.scale = scale;
    boundingBox.position = position;
    registry.boundingBoxes.insert(entity, boundingBox);
    registry.highlightables.emplace(entity);
    return entity;
}

Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                               const std::string& nextScene) {
    createEmptyButton(entity, position, scale, label);
    ChangeScene changeScene;
    changeScene.scene = nextScene;
    registry.changeScenes.insert(entity, changeScene);
    return entity;
}

/**
    - load entire sprite sheet as 1 SDL_Surface
    - create SDL_Rect for given sprite sheet frame
    - draw frame with SDL_RenderCopy()

    create sprite sheet

    when drawing
    - draw a section of it, what is the offset?
    - which animation are we drawing?
    - how do we know which anim to draw?
    - at each tick, how to draw 'next' frame?

    if == spirte sheet:
    play_animation
    - frames
    - offset w,h
    - cell w,h

    frames has to be hard coded
*
*/
Entity createSpriteSheet(const Entity& entity, vec2 position, uint sheetWidth, uint sheetHeight, uint cellWidth,
                         uint cellHeight) {
    SpriteSheet ss;
    ss.position = position;
    ss.sheetWidth = sheetWidth;
    ss.sheetHeight = sheetHeight;
    ss.cellWidth = cellWidth;
    ss.cellHeight = cellHeight;

    return entity;
}

void setZone(Entity entity, ZONE_TYPE zType, vec2 position) {
    Zone zone = registry.zones.emplace(entity);
    zone.position = position;
    zone.type = zType;
}