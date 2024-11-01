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
    return createEmptyButton(entity, position, scale, label, "button");
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName) {
    createSprite(entity, position, scale, 0, textureName);
    registry.interactables.emplace(entity);
    BoundingBox boundingBox;
    boundingBox.scale = scale;
    boundingBox.position = position;
    registry.boundingBoxes.insert(entity, boundingBox);
    registry.highlightables.emplace(entity);
    registry.buttons.emplace(entity);
    Text text;
    text.fontSize = 16;
    text.position = position;
    text.text = label;
    registry.texts.insert(entity, text);
    return entity;
}

Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName,
                               const std::string& nextScene) {
    createEmptyButton(entity, position, scale, label, textureName);
    ChangeScene changeScene;
    changeScene.scene = nextScene;
    registry.changeScenes.insert(entity, changeScene);
    return entity;
}

Entity createDashTheTurtle(const Entity& entity, vec2 position) { 
    Entity dash = createSprite(entity, position, {100, 100}, 0, "button");
    
    auto& motion = registry.motions.get(dash);
    motion.velocity = {0, 0};
    motion.collides = false;

    DashTheTurtle dashComponent;
    dashComponent.behavior = DASH_STATES::IDLE;
    dashComponent.nearestLightRayDirection = {1000000000, 1000000000};

    return entity;

Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName) {
    createEmptyButton(entity, position, scale, label, textureName);
    registry.resumeGames.emplace(entity);
    return entity;
}


void setZone(Entity entity, ZONE_TYPE zType, vec2 position) {
    Zone zone = registry.zones.emplace(entity);
    zone.position = position;
    zone.type = zType;
}