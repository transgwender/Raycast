#include "world_init.hpp"
#include "ecs/registry.hpp"
#include "systems/physics.hpp"
#include "systems/particles.hpp"
#include "utils/math.hpp"
#include "utils/mesh_utils.hpp"

Entity createSprite(const Entity& entity, const vec2 position, const vec2 scale, float angle,
                    const std::string& textureName, const Layer layer, const vec4 color) {
    auto& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;
    motion.angle = angle;

    registry.materials.insert(entity, {TEXTURED, get_tex(textureName), color, layer});

    return entity;
}

Entity createLight(const Entity& entity, vec2 position, float dir) {
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

    ParticleSpawner spawner;
    spawner.texture = texture_manager.getVirtual("light");
    spawner.initial_speed = 0.0f;
    spawner.spin_velocity = 0.0f;
    spawner.direction = vec2(0, 1);
    spawner.color = vec4(1.7, 1.7, 0, 1);
    spawner.spread = 0.0f;
    spawner.initial_scale = vec2(8, 8);
    spawner.scale_change = -15.f;
    spawner.alpha_change = -1.7f;
    spawner.lifetime = 1;
    spawner.max_particles = 8;
    registry.particleSpawners.insert(light, spawner);

    return entity;
}

Entity createMirror(const Entity& entity, const Mirror& mirror) {
    vec2 scale = vec2({5, 40});

    // Draw mirror attachment
    OnLinearRails rails;
    Rotatable rotatable;
    const auto attachment = Entity();
    if (mirror.mirrorType == "rotate") {
        createSprite(attachment, mirror.position, vec2(12, 12), mirror.angle, "gear");
    } else { // Assume only other mirror type is a RAIL
        rails.length = mirror.railLength;
        rails.angle = mirror.railAngle;
        createSprite(attachment, mirror.position, vec2(mirror.railLength * 2, 3), mirror.railAngle, "mirror_0");
    }

    // Draws actual mirror BEFORE registering rails/rotate
    createSprite(entity, mirror.position, scale, mirror.angle, "mirror_1");

    if (mirror.mirrorType == "rotate") {
        rotatable.snap_angle = mirror.snap_angle;
        registry.rotatable.insert(entity, rotatable);
    } else {
        rails.snap_segments = mirror.snap_segments;
        initLinearRails(entity, rails);
    }

    registry.reflectives.emplace(entity);

    // Initialize collider
    registry.collideables.emplace(entity);
    registry.colliders.emplace(entity);
    auto& collider = registry.colliders.get(entity);
    collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;
    // collider.user_interaction_bounds_type = BOUNDS_TYPE::RADIAL;
    collider.user_interaction_bounds_type = BOUNDS_TYPE::RECTANGULAR;
    collider.width = scale.x;
    collider.height = scale.y;

    return entity;
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label) {
    return createEmptyButton(entity, position, scale, label, "button");
}

Entity createEmptyButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                         const std::string& textureName, vec3 color) {
    createSprite(entity, position, scale, 0, textureName, UI_FOREGROUND);
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
    if (!label.empty()) {
        Text text;
        text.size = 57;
        text.position = position;
        text.text = label;
        text.color = color;
        text.layer = UI_TEXT;
        text.centered = true;
        registry.texts.insert(entity, text);
    }
    return entity;
}

Entity createChangeSceneButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                               const std::string& textureName, const std::string& nextScene, vec3 color) {
    createEmptyButton(entity, position, scale, label, textureName, color);
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

Entity createResumeButton(const Entity& entity, vec2 position, vec2 scale, const std::string& label,
                          const std::string& textureName) {
    createEmptyButton(entity, position, scale, label, textureName, {255, 255, 255});
    registry.resumeGames.emplace(entity);
    return entity;
}

// imageHeight and imageWidth control the actual dimensions of the sprite when rendered onto the screen
Entity createSpriteSheet(const Entity& entity, vec2 position, float sheetWidth, float sheetHeight, float cellWidth,
                         float cellHeight, const std::vector<unsigned int>& animationFrames,
                         const std::string& textureName, float imageWidth, float imageHeight, const vec4& color, float angle) {
    SpriteSheet ss;
    ss.position = position;
    ss.sheetWidth = sheetWidth;
    ss.sheetHeight = sheetHeight;
    ss.cellWidth = cellWidth;
    ss.cellHeight = cellHeight;
    ss.animationFrames = animationFrames;

    if (imageWidth == 0 || imageHeight == 0) {
        createSprite(entity, position, vec2(cellWidth, cellHeight), angle, textureName, FOREGROUND, color);
    } else {
        createSprite(entity, position, vec2(imageWidth, imageHeight), angle, textureName, FOREGROUND, color);
    }

    registry.spriteSheets.insert(entity, ss);
    return entity;
}

void setZone(const Entity& entity, ZONE_TYPE zType, vec2 position) {
    Zone zone{};
    zone.position = position;
    zone.type = zType;

    registry.zones.insert(entity, zone);
    if (zType == ZONE_TYPE::START) {
        const std::vector<unsigned int> frames = {7};
        createSpriteSheet(entity, position, 112, 16, 16, 16, frames, "start_gem_spritesheet");
    } else if (zType == ZONE_TYPE::END) {
        const std::vector<unsigned int> frames = {7};
        createSpriteSheet(entity, position, 112, 16, 16, 16, frames, "end_plant_big");
    }
}

void initLinearRails(const Entity& entity, OnLinearRails rails) {
    Motion& motion = registry.motions.get(entity);
    auto direction = vec2(cos(rails.angle), sin(rails.angle));
    vec2 firstEndpoint = motion.position + rails.length * direction;
    vec2 secondEndpoint = motion.position - rails.length * direction;
    rails.firstEndpoint = firstEndpoint;
    rails.secondEndpoint = secondEndpoint;
    rails.direction = direction;
    registry.entitiesOnLinearRails.insert(entity, rails);
}

// state describes how far the lever has been pushed
// effect is the effect that the lever has on the affectedEntity when activated
// activeLever is the state required to activate it

Entity createLever(const Entity& affectedEntity, const vec2& position, LEVER_STATES state, LEVER_EFFECTS effect,
                   LEVER_STATES activeLever) {

    Entity leverEntity = Entity();
    std::vector<unsigned int> vec = {6};
    Entity ssEntity = createSpriteSheet(leverEntity, position, 192, 32, 32, 32, {6}, "lever_sprite_sheet", 20, 20);
    SpriteSheet& ss = registry.spriteSheets.get(ssEntity);
    ss.currFrame = (unsigned int)state;
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

void initMesh(const Entity& entity, const std::string& mesh_name, const vec2& position, const float angle,
              const vec2& scale) {
    const auto filename = mesh_path(mesh_name);
    // Load mesh
    Mesh mesh{};
    MeshUtils::loadFromOBJFile(filename, mesh.vertices, mesh.vertex_indices, mesh.original_size);
    registry.meshes.insert(entity, mesh);

    // Initialize mesh motion
    Motion& mesh_motion = registry.motions.emplace(entity);
    mesh_motion.position = position;
    mesh_motion.angle = angle;
    mesh_motion.scale = scale * mesh.original_size;

    // Initialize mesh collider
    Collider& collider = registry.colliders.emplace(entity);
    collider.angle = angle;
    collider.bounds_type = BOUNDS_TYPE::MESH;
    collider.height = mesh_motion.scale.y;
    collider.width = mesh_motion.scale.x;
    collider.user_interaction_bounds_type = BOUNDS_TYPE::MESH;
}

unsigned int world_init::portal_color_i = 0;

void createPortals(const vec2 pos1, const float angle1, const vec2 pos2, const float angle2) {
    vec2 portal_size = vec2(13, 42);

    const world_init w;
    const vec4 color = w.portal_colors[world_init::portal_color_i % 8];

    const Entity portal_1 = createPortalEntity(pos1, angle1, portal_size, color);
    const Entity portal_2 = createPortalEntity(pos2, angle2, portal_size, color);

    // Increment static portal color counter to generate next color
    world_init::portal_color_i += 1;

    linkPortals(portal_1, portal_2, pos1, angle1, pos2, angle2, color);
}

Entity createPortalEntity(const vec2 position, const float angle, const vec2& size, const vec4& color) {
    const Entity portal = Entity();

    // Insert portal into registry and create sprite
    createSpriteSheet(portal, position, 512.f, 64.f, 64.f, 64.f, {8}, "portal_ss", 58, 58, color, angle);
    registry.collideables.emplace(portal);

    // Configure and add the collider
    Collider collider{};
    collider.angle = angle;
    collider.width = size.x;
    collider.height = size.y;
    collider.bounds_type = BOUNDS_TYPE::RECTANGULAR;

    registry.colliders.insert(portal, collider);

    return portal;
}

void linkPortals(const Entity& portal_1, const Entity& portal_2, const vec2& pos1, const float angle1, const vec2& pos2, const float angle2, const vec4& color) {
    Portal p1{};
    Portal p2{};

    p1.other_portal = portal_2;
    p1.position = pos1;
    p1.angle = angle1;

    p2.other_portal = portal_1;
    p2.position = pos2;
    p2.angle = angle2;

    registry.portals.insert(portal_1, p1);
    registry.portals.insert(portal_2, p2);

    ParticleSystem::createPortalParticles(p1, color);
    ParticleSystem::createPortalParticles(p2, color);
}

