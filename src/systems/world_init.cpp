#include "world_init.hpp"
#include "utils/math.hpp"
#include "systems/physics.hpp"
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

Entity createLight(const Entity &entity, vec2 position, float dir) {
    vec2 scale = vec2({8, 8});
    vec2 velocity = raycast::math::from_angle(dir);
    velocity = raycast::math::set_mag(velocity, PhysicsSystem::SpeedOfLight);
    Entity light = createSprite(entity, position, scale, dir, "light");

    // Initialize collider
    registry.collideables.emplace(light);
    registry.colliders.emplace(light);
    auto& collider = registry.colliders.get(light);
    collider.bounds_type = BOUNDS_TYPE::RADIAL;
    collider.width = scale.x;
    collider.height = scale.y;

    registry.lightRays.emplace(light);

    auto& motion = registry.motions.get(light);
    motion.velocity = velocity;
    motion.angle = dir;

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

    // Initialize collider
    registry.collideables.emplace(entity);
    registry.colliders.emplace(entity);
    auto& collider = registry.colliders.get(entity);
    collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;
    collider.user_interaction_bounds_type = BOUNDS_TYPE::RADIAL;
    collider.width = scale.x;
    collider.height = scale.y;


    return entity;
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label) {
    return createEmptyButton(entity, position, scale, label, "button");
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName) {
    createSprite(entity, position, scale, 0, textureName);
    if (!registry.interactables.has(entity)) {
        registry.interactables.emplace(entity);
    }
    if (!registry.colliders.has(entity)) {
        Collider& collider = registry.colliders.emplace(entity);
        collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;
        collider.width = scale.x;
        collider.height = scale.y;
        collider.needs_update = true;
    }
    if (!registry.highlightables.has(entity)) {
        registry.highlightables.emplace(entity);
    }
    if (!registry.buttons.has(entity)) {
        registry.buttons.emplace(entity);
    }
    Text text;
    text.size = 64;
    text.position = position;
    text.text = label;
    text.centered = true;
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
    Entity dash = createSprite(entity, position, {10, 10}, 0, "button");

    auto& motion = registry.motions.get(dash);
    motion.velocity = {0, 0};
    motion.scale = {10, 10};

    registry.collideables.emplace(entity);
    auto& collider = registry.colliders.emplace(entity);

    // TEMPORARY
    collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;
    collider.width = 10;
    collider.height = 10;
    // motion.collides = false;

    DashTheTurtle dashComponent;
    dashComponent.behavior = DASH_STATES::IDLE;
    dashComponent.nearestLightRayDirection = {1000000000, 1000000000};
    registry.turtles.emplace(dash);

    return entity;
}

Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label, const std::string& textureName) {
    createEmptyButton(entity, position, scale, label, textureName);
    registry.resumeGames.emplace(entity);
    return entity;
}

// imageHeight and imageWidth control the actual dimensions of the sprite when rendered onto the screen

Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth,
                         float cellHeight, const std::vector<unsigned int>& animationFrames,
                         const std::string textureName, float imageWidth, float imageHeight) {
    SpriteSheet ss;
    ss.position = position;
    ss.sheetWidth = sheetWidth;
    ss.sheetHeight = sheetHeight;
    ss.cellWidth = cellWidth;
    ss.cellHeight = cellHeight;
    ss.animationFrames = animationFrames;

    if (imageWidth == 0 || imageHeight == 0) {
        createSprite(entity, position, vec2(cellWidth, cellHeight), 0, textureName);
    } else {
        createSprite(entity, position, vec2(imageWidth, imageHeight), 0, textureName);
    }

    // TODO: add scale handling to flip direction of turtle when moving
    registry.spriteSheets.insert(entity, ss);
    return entity;
}

void setZone(Entity entity, ZONE_TYPE zType, vec2 position) {
    Zone zone = registry.zones.emplace(entity);
    zone.position = position;
    zone.type = zType;
}


// state describes how far the lever has been pushed
// effect is the effect that the lever has on the affectedEntity when activated
// activeLever is the state required to activate it

Entity createLever(Entity affectedEntity, const vec2& position, LEVER_STATES state, LEVER_EFFECTS effect,
    LEVER_STATES activeLever) {

    Entity leverEntity = Entity();
    std::vector<unsigned int> vec = {6};
    createSpriteSheet(leverEntity, position, 192, 32, 32, 32, {6}, "lever_sprite_sheet", 20, 20);
    Lever lever{};
    lever.state = state;
    lever.movementState = LEVER_MOVEMENT_STATES::STILL;
    lever.effect = effect;
    lever.activeLever = activeLever;
    lever.affectedEntity = affectedEntity;
    registry.levers.insert(leverEntity, lever);
    registry.collideables.emplace(leverEntity);
    Collider collider{};
    collider.width = 12;
    collider.height = 20;
    collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;
    registry.colliders.insert(leverEntity, collider);
    Zone zone{};
    zone.position = position;
    zone.type = ZONE_TYPE::ZONE_TYPE_COUNT;
    registry.zones.insert(leverEntity, zone);
    return leverEntity;



}