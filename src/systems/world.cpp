#include "world.hpp"

#include "collisions.h"
#include "world_init.hpp"

#include <SDL.h>
#include <cassert>
#include <fstream>
#include <iostream>

#include "components_json.hpp"
#include "glm/detail/func_trigonometric.inl"
#include "logging/log.hpp"
#include "particles.hpp"
#include "systems/menu.hpp"
#include "systems/physics.hpp"
#include "utils/defines.hpp"
#include "utils/math.hpp"

#include <utils.h>

// create the light-maze world
WorldSystem::WorldSystem() : next_light_spawn(0.f) {
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
    // Destroy music components
    sounds.free_sounds();

    // Destroy all created components
    registry.clear_all_components();

    // Close the window
    glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char* desc) { LOG_ERROR("{}: {}", error, desc); }
} // namespace

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow* WorldSystem::create_window() {
    // Initialize GLFW
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return nullptr;
    }

    // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3
    // and enable the glDebugMessageCallback to have OpenGL catch your mistakes
    // for you. GLFW / OGL Initialization
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, 0);

    // Create the main window (for rendering, keyboard, and mouse input)
    window = glfwCreateWindow(window_width_px, window_height_px, "Raycast", nullptr, nullptr);
    if (window == nullptr) {
        LOG_ERROR("Failed to glfwCreateWindow");
        return nullptr;
    }

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(window, this);
    auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
    };
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
    };
    auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) {
        double xpos, ypos;
        glfwGetCursorPos(wnd, &xpos, &ypos);
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(_0, _1, _2, xpos, ypos);
    };

    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);

    SoundSystem::init();

    return window;
}

void WorldSystem::init(PersistenceSystem *persistence_ptr) {
    this->persistence = persistence_ptr;
    menus.init(persistence_ptr);
    sounds.change_volume_music(persistence->get_settings_music_volume());
    sounds.change_volume_sfx(persistence->get_settings_sfx_volume());
    scenes.init(scene_state_entity, persistence_ptr);
    sounds.load_all_sounds();

    // Set all states to default
    restart_game();
}

bool WorldSystem::isInLevel() {
    assert(registry.levels.size() <= 1);
    assert(registry.endLevels.size() <= 1);
    return (!registry.levels.components.empty() || !registry.endLevels.components.empty());
}

bool WorldSystem::shouldStep() {
    if (menus.is_menu_open()) {
        return !registry.menus.components.front().shouldBlockSteps;
    }

    return true;
}

bool WorldSystem::shouldAllowInput() {
    if (menus.is_menu_open()) {
        return !registry.menus.components.front().shouldBlockInput;
    }
    return true;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
    if(do_restart) {
        do_restart = false;
        restart_game();
    }

    // update frame rate value
    const int fps_value = Utils::fps(elapsed_ms_since_last_update);
    registry.texts.get(frame_rate_entity).text = !frame_rate_enabled ? "" : "FPS: " + std::to_string(fps_value);

    if (isInLevel() && shouldStep()) {
        // ECSRegistry& test_registry = registry;
        next_light_spawn -= elapsed_ms_since_last_update * current_speed;

        for (int i = 0; i < registry.levers.components.size(); i++) {
            // auto& leverEntity = registry.levers.entities[i];
            auto& lever = registry.levers.components[i];
            if ((int) lever.state == (int) lever.activeLever) {
                if (lever.effect == LEVER_EFFECTS::REMOVE) {
                    if (!registry.invisibles.has(lever.affectedEntity)) {
                        registry.invisibles.emplace(lever.affectedEntity);
                    }
                    //registry.remove_all_components_of(lever.affectedEntity);
                }
            } else {
                if (lever.effect == LEVER_EFFECTS::REMOVE) {
                    if (registry.invisibles.has(lever.affectedEntity)) {
                        registry.invisibles.remove(lever.affectedEntity);
                    }
                    // registry.remove_all_components_of(lever.affectedEntity);
                }
            }
        }

        for (int i = 0; i < registry.lightRays.components.size(); i++) {
            auto& lightEntity = registry.lightRays.entities[i];
            auto& light = registry.lightRays.components[i];
            if (light.last_reflected_timeout > 0)
                light.last_reflected_timeout -= elapsed_ms_since_last_update;
            Motion& motion = registry.motions.get(lightEntity);
            if (motion.position.x < -10 || motion.position.x > native_width + 10 || motion.position.y < -10 ||
                motion.position.y > native_height + 10) {
                registry.remove_all_components_of(lightEntity);
            }
        }

        for (int i = 0; i < registry.minisuns.components.size(); i++) {
            auto& minisunEntity = registry.minisuns.entities[i];
            auto& minisun = registry.minisuns.components[i];
            if (registry.litEntities.has(minisunEntity)) {
                LightUp &light = registry.litEntities.get(minisunEntity);
                if (light.counter_ms > 0) {
                    light.counter_ms -= elapsed_ms_since_last_update;
                } else {
                    minisun.light_level_percentage = clamp(minisun.light_level_percentage - 0.01f, 0.0f, 1.0f);
                    if (minisun.light_level_percentage <= 0.0f) {
                        registry.litEntities.remove(minisunEntity);
                        minisun.lit = false;
                    } else {
                        light.counter_ms = LIGHT_TIMER_MS;
                    }
                } 
            }
        }

        if (registry.lightRays.components.size() < MAX_LIGHT_ON_SCREEN && next_light_spawn < 0.f) {
            // reset timer
            next_light_spawn = LIGHT_SPAWN_DELAY_MS;

            auto& sources = registry.lightSources.entities;

            for (int i = 0; i < sources.size(); i++) {

                if (registry.endCutsceneCounts.size() > 0) {
                    auto &endCutscene = registry.endCutsceneCounts.components.front();
                    if (++endCutscene.lightCount > endCutscene.maxInclusive) {
                        break;
                    }
                }

                Zone& zone = registry.zones.get(sources[i]);
                vec2 position = zone.position;
                float angle = registry.lightSources.components[i].angle;

                const auto entity = Entity();
                createLight(entity, position, angle);
            }
        }

        for (int i = 0; i < registry.gravities.components.size(); i++) {
            auto& gravityEntity = registry.gravities.entities[i];
            auto& motion = registry.motions.get(gravityEntity);
            motion.velocity.y += gravity;
        }

        for (auto& lever : registry.levers.entities) {
            Lever& l = registry.levers.get(lever);
            if (l.sfx_timer >= 0.f) {
                l.sfx_timer -= elapsed_ms_since_last_update;
            }
        }

        updateDash();
    }

    return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
    input_manager.active_entities.clear();
    // Debugging for memory/component leaks
    registry.list_all_components();
    LOG_INFO("Restarting game state");

    // Reset the game speed
    current_speed = 1.f;

    // Reset light respawn timer
    next_light_spawn = 0.f;

    // Remove all entities that we created
    registry.clear_all_components();

    // Debugging for memory/component leaks
    registry.list_all_components();

    // Parse scene file
    if (registry.scenes.has(scene_state_entity)) {
        scenes.try_parse_scene(registry.scenes.get(scene_state_entity).scene_tag);
    } else {
        LOG_ERROR("Hmm, there should have been a scene state entity defined.");
    }

    if (!registry.levelSelects.components.empty()) {
        menus.generate_level_select_buttons((int)scenes.level_count());
    }

    if (!registry.endCutsceneCounts.components.empty()) {
        sounds.stop_music();
    } else {
        sounds.play_background();
    }

    // add frame counter
    frame_rate_entity = Entity();
    registry.texts.insert(frame_rate_entity, {"", {1, 5}, 32, vec4(255.0), UI_TEXT, false});
}

void WorldSystem::change_scene(std::string& scene_tag) {
    Scene& scene = registry.scenes.get(scene_state_entity);
    scene.scene_tag = scene_tag;
    do_restart = true;
//    restart_game(); // TODO: Change to function for changing scene specifically
}

// Handle collisions between entities
void WorldSystem::handle_collisions() {
    // registry.collisions.emplace_with_duplicates(Entity(), Entity());
    // Loop over all collisions detected by the physics system
    auto& collisionsRegistry = registry.collisions;
    for (int i = 0; i < collisionsRegistry.entities.size(); i++) {
        if (registry.portals.has(collisionsRegistry.entities[i]) &&
            registry.lightRays.has(collisionsRegistry.components[i].other)) {
            handle_portal_collisions(collisionsRegistry.entities[i], collisionsRegistry.components[i].other);
            continue;
        }

        // Ignore Turtle to Light ray collision
        if (registry.turtles.has(collisionsRegistry.entities[i]) &&
            !registry.lightRays.has(collisionsRegistry.components[i].other)) {
            handle_turtle_collisions(i);
            continue;
        }
        // for now, only handle collisions involving light ray as other object
        if (!registry.lightRays.has(collisionsRegistry.components[i].other) ||
            registry.lightRays.has(collisionsRegistry.entities[i]))
            continue;
        if (registry.reflectives.has(collisionsRegistry.entities[i])) {
            handle_reflection(collisionsRegistry.entities[i], collisionsRegistry.components[i].other,
                              collisionsRegistry.components[i].side, collisionsRegistry.components[i].overlap);
        } else {
            
            if (registry.minisuns.has(collisionsRegistry.entities[i])) {
                handle_minisun_collision(collisionsRegistry.entities[i]);
            } else if (registry.endCutsceneCounts.has(collisionsRegistry.entities[i])) {
                handle_end_cutscene_collision(collisionsRegistry.entities[i]);
            }
            handle_non_reflection(collisionsRegistry.entities[i], collisionsRegistry.components[i].other);
        }
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

void WorldSystem::handle_minisun_collision(Entity& minisun_entity) {
    if (!registry.litEntities.has(minisun_entity)) {
        auto &minisun = registry.minisuns.get(minisun_entity);
        minisun.lit = true;
        LightUp l;
        minisun.light_level_percentage = 0.4f;
        registry.litEntities.insert(minisun_entity, l);
    } else {
        LightUp& minisun_light = registry.litEntities.get(minisun_entity);
        minisun_light.counter_ms = LIGHT_TIMER_MS;
        auto& minisun = registry.minisuns.get(minisun_entity);
        minisun.light_level_percentage = clamp(minisun.light_level_percentage + 0.4f, 0.0f, 1.0f);

    }
}

void WorldSystem::handle_end_cutscene_collision(Entity& end_cutscene_count_entity) {
    auto &end_cutscene = registry.endCutsceneCounts.get(end_cutscene_count_entity);
    std::string next_tag = "end_" + std::to_string(++end_cutscene.sequence);
    scenes.reload_background(next_tag);
    if (end_cutscene.sequence == 3) {
        sounds.play_forest();
    }
    if (end_cutscene.sequence == end_cutscene.maxInclusive) {
        registry.texts.insert(Entity(), {"Thank you for playing our game", {20, 20}, 36, vec4(255.0), UI_TEXT, false});
        registry.texts.insert(Entity(), {"Press any button to return to the main menu", {180, 20}, 36, vec4(255.0), UI_TEXT, false});
    }
}

// When colliding entities to not reflect, if one of the entities
// is a light ray and the other is an end zone, win the level
// if the other entity is not an end-zone, destroy light ray
// Invariant: other is a light ray
void WorldSystem::handle_non_reflection(Entity& collider, Entity& other) {
    assert(registry.lightRays.has(other));
    if (registry.zones.has(collider)) {
        switch (registry.zones.get(collider).type) {
        case ZONE_TYPE::END: {
            LOG_INFO("Level beaten!");
            //            std::string next_scene = "gamefinish";
            //            change_scene(next_scene);
            if (registry.levels.size() > 0) {
                assert(registry.levels.size() == 1);
                Level& level = registry.levels.components.front();
                if (registry.menus.components.empty()) {
                    if (level.id == scenes.level_count()) {
                        std::string tag = "end1";
                        change_scene(tag);
                    } else {
                        menus.generate_level_win_popup(level.id, (int)scenes.level_count());
                        persistence->set_beaten(level.id);
                        persistence->set_accessible(level.id + 1);
                        persistence->try_write_save();
                        sounds.play_sound("win.wav");
                    }
                }
            } else if (registry.endLevels.size() > 0) {
                assert(registry.endLevels.size() == 1);
                EndLevel& endLevel = registry.endLevels.components.front();
                std::string tag = "end" + std::to_string(endLevel.id+1);
                change_scene(tag);
            }
            registry.remove_all_components_of(other);
            break;
        }
        case ZONE_TYPE::START: {
            return;
        }
        default: {
            sounds.play_sound("light-collision.wav");
            ParticleSystem::createLightDissipation(registry.motions.get(other));
            registry.remove_all_components_of(other);
            break;
        }
        }
    } else {
        if (!registry.turtles.has(collider)) {
            sounds.play_sound("light-collision.wav");
            registry.remove_all_components_of(other);
        }
    }
}

// Reflect light ray based on collision normal
// Invariant: other is a light ray
// Side: 1 if y side, 2 if x side
void WorldSystem::handle_reflection(Entity& reflective, Entity& reflected, int side, float overlap) {
    assert(registry.lightRays.has(reflected));
    Light& light = registry.lightRays.get(reflected);
    // don't reflect off of same mirror twice
    // due to being inside
    if (light.last_reflected == reflective && light.last_reflected_timeout > 0) {
        return;
    }

    if (registry.inOrbits.has(reflected)) {
        registry.inOrbits.get(reflected).bodyOfMassJustOrbited = Entity(); // When collision with a mirror happens, reset last body of mass orbitted
    }

    Motion& light_motion = registry.motions.get(reflected);
    Motion& reflective_surface_motion = registry.motions.get(reflective);
    float angle_addition = M_PI_2;
    if (side == 1) {
        // LOG_INFO("y-Side reflection\n");
        angle_addition = M_PI_2;
    }
    if (side == 2) {
        // LOG_INFO("x-Side reflection\n");
        angle_addition = 0.f;
    }
    vec2 reflective_surface_normal = {cos(reflective_surface_motion.angle + angle_addition),
                                      sin(reflective_surface_motion.angle + angle_addition)};
    float angle_between = atan2(reflective_surface_normal.y, reflective_surface_normal.x) -
                          atan2(light_motion.velocity.y, light_motion.velocity.x);
    vec2 reflected_velocity = -light_motion.velocity +
                              2.f * dot(light_motion.velocity, reflective_surface_normal) * reflective_surface_normal;
    // Check whether reflected velocity points towards centre of rectangle — bad reflection!
    vec2 light_to_reflective = vec2(light_motion.position - reflective_surface_motion.position);
    if (dot(reflected_velocity, light_to_reflective) < dot(light_motion.velocity, light_to_reflective)) {
        LOG_INFO("Reflection edge case -- abort reflection\n");
        return;
    }
    // Play reflection sound
    sounds.play_sound("light-collision.wav");

    // Correct light position with overlap
    light_motion.position -= normalize(light_motion.velocity) * overlap;

    // Update motion
    light_motion.velocity = reflected_velocity;
    light.last_reflected = reflective;
    light.last_reflected_timeout = DOUBLE_REFLECTION_TIMEOUT;
    angle_between = atan2(reflective_surface_normal.y, reflective_surface_normal.x) -
                    atan2(light_motion.velocity.y, light_motion.velocity.x);
    light_motion.angle -= 2 * angle_between;
}

// if the turtle collides against a wall, stop the turtle from moving further
void WorldSystem::handle_turtle_collisions(int i) {

    auto& collisionsRegistry = registry.collisions;
    Entity turtle = collisionsRegistry.entities[i];
    Entity other = collisionsRegistry.components[i].other;

    // TODO: Rough patch to handle TRIPLE COLLISIONS... might want to improve in the future
    if (!registry.motions.has(other) || !registry.colliders.has(other)) {
        return;
    }
    Motion& turtle_motion = registry.motions.get(turtle);
    Collider& turtle_collider = registry.colliders.get(turtle);
    Motion& barrier_motion = registry.motions.get(other);
    Collider& barrier_collider = registry.colliders.get(other);

    float turtleLeft = turtle_motion.position.x - turtle_collider.width / 2;
    float turtleRight = turtle_motion.position.x + turtle_collider.width / 2;
    float turtleTop = turtle_motion.position.y - turtle_collider.height / 2;
    float turtleBottom = turtle_motion.position.y + turtle_collider.height / 2;

    float barrierLeft = barrier_motion.position.x - barrier_collider.width / 2;
    float barrierRight = barrier_motion.position.x + barrier_collider.width / 2;
    float barrierTop = barrier_motion.position.y - barrier_collider.height / 2;
    float barrierBottom = barrier_motion.position.y + barrier_collider.height / 2;


    float overlapX = std::min(turtleRight, barrierRight) - std::max(turtleLeft, barrierLeft);
    float overlapY = std::min(turtleBottom, barrierBottom) - std::max(turtleTop, barrierTop);


    // Check to see collision penetration, is there more overlap on the x-axis or the y-axis? Resolve the collision on the axis with the most overlap
    if (abs(overlapX) < abs(overlapY)) {
         if (turtle_motion.position.x < barrier_motion.position.x) {
            // If the "barrier" is a lever, push it to the right!
            if (registry.levers.has(other)) {
                Lever& l = registry.levers.get(other);
                if (l.sfx_timer <= 0.f) {
                    sounds.play_sound("lever.wav");
                    l.sfx_timer = sounds.lever_sfx_duration_ms;
                }

                if (registry.motions.get(turtle).velocity.x > 0) {
                    registry.levers.get(other).state = LEVER_STATES::RIGHT;
                } else {
                    registry.levers.get(other).state = LEVER_STATES::LEFT;
                }
            }
        }
         if (turtle_motion.position.x > barrier_motion.position.x) {
            // If the "barrier" is a lever, push it to the left!
            if (registry.levers.has(other)) {
                Lever& l = registry.levers.get(other);
                if (l.sfx_timer <= 0.f) {
                    sounds.play_sound("lever.wav");
                    l.sfx_timer = sounds.lever_sfx_duration_ms;
                }

                if (registry.motions.get(turtle).velocity.x > 0) {
                    registry.levers.get(other).state = LEVER_STATES::RIGHT;
                } else {
                    registry.levers.get(other).state = LEVER_STATES::LEFT;
                }
            }
        }

    } else {
        if (turtle_motion.position.y < barrier_motion.position.y) {
            turtle_motion.position.y =
                barrier_motion.position.y - abs(barrier_collider.height / 2) - abs(turtle_collider.height / 2) - 0.01f;
        }
        if (turtle_motion.position.y > barrier_motion.position.y) {
            turtle_motion.position.y =
                barrier_motion.position.y + abs(barrier_collider.height / 2) + abs(turtle_collider.height / 2) + 0.01f;
        }
        turtle_motion.velocity.y = 0;
    }

    // Move to the left of barrier
    //if (turtle_motion.position.x < barrier_motion.position.x) {
    //    // registry.sprite
    //    turtle_motion.position.x = barrier_motion.position.x - barrier_motion.scale.x / 2 - abs(turtle_collider.width / 2);
    //}
    //if (turtle_motion.position.x > barrier_motion.position.x) {
    //    turtle_motion.position.x =
    //        barrier_motion.position.x + barrier_motion.scale.x / 2 + abs(turtle_collider.width / 2);
    //}
}

void WorldSystem::handle_portal_collisions(Entity& portal, Entity& light) {
    auto light_motion = registry.motions.get(light);
    auto enter_portal = registry.portals.get(portal);
    auto exit_portal = registry.portals.get(enter_portal.other_portal);

    // Calculate portal normal direction
    vec2 portal_normal = {cos(enter_portal.angle), sin(enter_portal.angle)};

    // Normalize light's velocity to get its direction
    vec2 light_direction = normalize(light_motion.velocity);

    // Check if the light collided with the backside of the portal
    float dot_product = dot(light_direction, portal_normal);
    if (dot_product > 0) {
        // Light collided with the backside
        sounds.play_sound("light-collision.wav");
        registry.remove_all_components_of(light);
        return;
    }

    // Calculate position offset
    float offset_length = 15;
    vec2 exit_position_offset = {
        cos(exit_portal.angle) * offset_length,
        sin(exit_portal.angle) * offset_length
    };

    // Calculate angle offset
    float velocity_angle = atan2(light_motion.velocity.y, light_motion.velocity.x);
    float exit_angle_offset = M_PI - enter_portal.angle + velocity_angle;

    // Remove and create new light at exit portal position
    registry.remove_all_components_of(light);
    createLight(light, exit_portal.position + exit_position_offset, exit_portal.angle + exit_angle_offset);
    sounds.play_sound("portal_long.wav", 0.25);
}


// Should the game be over?
bool WorldSystem::is_over() const { return bool(glfwWindowShouldClose(window)); }

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    input_manager.update_key_state(key, action, mod);

    if (registry.endCutsceneCounts.size() > 0) {
        EndCutsceneCount endCutscene = registry.endCutsceneCounts.components.front();
        if (endCutscene.sequence == endCutscene.maxInclusive) {
            if (action == 0) {
                std::string tag = "mainmenu";
                change_scene(tag);
            }
        }
        return;
    }

    // Resetting game
    if (IS_RELEASED(GLFW_KEY_R)) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        restart_game();
    }

    if (IS_PRESSED(GLFW_KEY_M)) {
        if (Mix_PausedMusic()) {
            Mix_ResumeMusic();
        } else {
            Mix_PauseMusic();
        }
    }


    // Pausing
    if (IS_RELEASED(GLFW_KEY_ESCAPE)) {
        assert(registry.menus.size() <= 1);
        if (registry.menus.size() == 0) {
            assert(registry.menus.size() <= 1);
            if (registry.levels.size() > 0) {
                Level& level = registry.levels.components.front();
                menus.generate_pause_popup(level.id);
            }
        } else {
            Menu& menu = registry.menus.components.front();
            if (menu.canClose) {
                menus.try_close_menu();
            }
        }
    }

    if (IS_RELEASED(GLFW_KEY_F)) {
        frame_rate_enabled = !frame_rate_enabled;
    }

    // Control the current speed with `<` `>`
    if (IS_RELEASED_WITH_SHIFT(GLFW_KEY_COMMA)) {
        current_speed -= 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    if (IS_RELEASED_WITH_SHIFT(GLFW_KEY_PERIOD)) {
        current_speed += 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    current_speed = fmax(0.f, current_speed);



#ifdef ALLOW_DEBUG_FUNCTIONS
    if (IS_PRESSED(GLFW_KEY_0)) {
        persistence->debug_set_all_accessible(scenes.level_count());
    }
#endif
}

void move_mirror(vec2 position) { registry.motions.get(registry.reflectives.entities[0]).position = position; }

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    input_manager.update_mouse_position(mouse_position);

    auto hovered_entities = input_manager.get_entities_at_mouse_pos();
    for (auto& highlightable : registry.highlightables.components) {
        highlightable.isHighlighted = false;
    }

    for (Entity entity : hovered_entities) {
        if (registry.highlightables.has(entity)) {
            if (shouldAllowInput() || registry.menuItems.has(entity)) {
                registry.highlightables.get(entity).isHighlighted = true;
            }
        }
    }

    if(shouldAllowInput()) {
        // handle the mirror movements
        for(auto entity : input_manager.active_entities) {
            Motion &clicked_motion = registry.motions.get(entity);
            vec2 to_mouse = clicked_motion.position - screenToWorld(mouse_position);

            if (registry.rotatable.has(entity)) { // rotatable mirrors
                // NOTE: we need to substract PI/2 since by default the mirror is perpendicular to +x-axis
                if (persistence->get_settings_hard_mode()) { // HARD MODE: No snapping
                    clicked_motion.angle = raycast::math::heading(to_mouse) - M_PI_2, registry.rotatable.get(entity);
                } else {
                    clicked_motion.angle = raycast::math::snap(raycast::math::heading(to_mouse) - M_PI_2, registry.rotatable.get(entity).snap_angle);
                }
            }

            if (registry.entitiesOnLinearRails.has(entity)) { // mirrors on rails
                OnLinearRails &rails = registry.entitiesOnLinearRails.get(entity);
                // ideally the fist endpoint should correspond to the left endpoint, but if we rotate
                // beyond the y-axis it flips so this check is necessary
                if (rails.firstEndpoint.x < rails.secondEndpoint.x) {
                    vec2 point;
                    if (persistence->get_settings_hard_mode()) { // HARD MODE: No snapping
                        point = {(screenToWorld(mouse_position) - input_manager.displacement_to_entity()).x,
                                 (screenToWorld(mouse_position) - input_manager.displacement_to_entity()).y};
                    } else {
                        point = {raycast::math::snap((screenToWorld(mouse_position) - input_manager.displacement_to_entity()).x, rails.length / rails.snap_segments),
                                  raycast::math::snap((screenToWorld(mouse_position) - input_manager.displacement_to_entity()).y, rails.length / rails.snap_segments)};
                    }
                    clicked_motion.position = raycast::math::clampToLineSegment(rails.firstEndpoint, rails.secondEndpoint, point);
                } else {
                    vec2 point;
                    if (persistence->get_settings_hard_mode()) { // HARD MODE: No snapping
                        point = {(screenToWorld(mouse_position) - input_manager.displacement_to_entity()).x,
                                  (screenToWorld(mouse_position) - input_manager.displacement_to_entity()).y};
                    } else {
                        point = {raycast::math::snap((screenToWorld(mouse_position) - input_manager.displacement_to_entity()).x, rails.length / rails.snap_segments),
                                  raycast::math::snap((screenToWorld(mouse_position) - input_manager.displacement_to_entity()).y, rails.length / rails.snap_segments)};
                    }
                    clicked_motion.position = raycast::math::clampToLineSegment(rails.secondEndpoint, rails.firstEndpoint, point);
                }
            }
        }
    }
}

void WorldSystem::on_mouse_button(int key, int action, int mod, double xpos, double ypos) {
    input_manager.update_mouse_button_state(key, action, mod, vec2(xpos, ypos));

    if (registry.endCutsceneCounts.size() > 0) {
        EndCutsceneCount endCutscene = registry.endCutsceneCounts.components.front();
        if (endCutscene.sequence == endCutscene.maxInclusive) {
            if (action == 0) {
                std::string tag = "mainmenu";
                change_scene(tag);
            }
        }
    }

    auto hovered_entities = input_manager.get_entities_at_mouse_pos();
    if (IS_RELEASED(GLFW_MOUSE_BUTTON_LEFT)) {
        // mouse initialized by clicked_entities
        for (const Entity& entity : hovered_entities) {
            assert(registry.motions.has(entity));
            if (registry.changeScenes.has(entity) && input_manager.active_entities.size() == 0) {
                sounds.play_sound("button-click.wav");
                ChangeScene& changeScene = registry.changeScenes.get(entity);
                change_scene(changeScene.scene);
                break;
            }
            if (registry.resumeGames.has(entity)) {
                sounds.play_sound("button-click.wav");
                menus.try_close_menu();
                break;
            }
            if (registry.deleteDatas.has(entity)) {
                auto &d = registry.deleteDatas.get(entity);
                auto &text = registry.texts.get(entity);
                if (d.isDoubleChecking) {
                    persistence->clear_data();
                    text.text = "Deleted";
                } else {
                    d.isDoubleChecking = true;
                    text.text = "Are you sure?";
                }
            }
            if (registry.volumeSliders.has(entity)) {
                auto &v = registry.volumeSliders.get(entity);
                auto &m = registry.motions.get(entity);
                auto &t = registry.materials.get(entity);
                float left = m.position.x - m.scale.x/2;
                int width, height;
                glfwGetWindowSize(window, &width, &height);
                float scaled = (xpos/width) * native_width;
                float value = (scaled - left)/m.scale.x;
                int index = floor(value * 26);
                if (index > 25) index = 25;
                std::string slider_texture = "slider" + std::to_string(index);
                t.texture = get_tex(slider_texture);
                if (v.setting == "music") {
                    persistence->set_settings_music_volume(value);
                    sounds.change_volume_music(value);
                } else if (v.setting == "sfx") {
                    persistence->set_settings_sfx_volume(value);
                    sounds.change_volume_sfx(value);
                }
            }
            if (registry.toggles.has(entity)) {
                auto &t = registry.toggles.get(entity);
                auto &m = registry.materials.get(entity);
                if (t.setting == "hard") {
                    persistence->set_settings_hard_mode(!persistence->get_settings_hard_mode());
                    int index = persistence->get_settings_hard_mode();
                    std::string toggle_texture = "toggle" + std::to_string(index);
                    m.texture = get_tex(toggle_texture);
                }
            }
        }
        input_manager.active_entities.clear();
    }

    if (IS_PRESSED(GLFW_MOUSE_BUTTON_LEFT)) {
        for (const Entity& entity : hovered_entities) {
            if (registry.entitiesOnLinearRails.has(entity) || registry.rotatable.has(entity)) {
                input_manager.active_entities.push_back(entity);
            }
        }
        if (hovered_entities.size() > 0) {
            input_manager.update_mouse_to_entity_displacement(registry.motions.get(hovered_entities[0]).position);
        }
    }
}

void WorldSystem::updateDash() {
    for (const Entity& dashEntity : registry.turtles.entities) {
        DASH_STATES dash_state = registry.turtles.get(dashEntity).behavior;
        vec2 ray = registry.turtles.get(dashEntity).nearestLightRayDirection;
        Motion& dm = registry.motions.get(dashEntity);

        if (registry.spriteSheets.components.empty()) {
            throw std::runtime_error("Error: Sprite sheet does not exist. Please make sure to add the sprite sheet to the level JSON.");
        }

        SpriteSheet& ss = registry.spriteSheets.get(dashEntity);
        Motion& ss_motion = registry.motions.get(dashEntity);

        if (dash_state == DASH_STATES::WALK) {
            // vec2 displacement = {(dm.position.x - ray.x), (dm.position.y - ray.y)};
            ss.currState = static_cast<unsigned int>(DASH_STATES::WALK);
            if (ray.x > 0) {
                dm.velocity.x = -dashSpeed;
                if (ss_motion.scale.x > 0) {
                    ss_motion.scale.x = -ss_motion.scale.x;  // Flip if currently positive
                }
            } else if (ray.x < 0) {
                dm.velocity.x = dashSpeed;
                if (ss_motion.scale.x < 0) {
                    ss_motion.scale.x = -ss_motion.scale.x;  // Flip if currently negative
                }
            }

            // if (dm.position.x < 100) {
            //     int w = 0;
            //     w++;
            // }

        } else if (dash_state == DASH_STATES::STARE) {
            dm.velocity = {0, 0};
            ss.currState = static_cast<unsigned int>(DASH_STATES::STARE);
        } else if (dash_state == DASH_STATES::IDLE) {
            dm.velocity = {0, 0};
            ss.currState = static_cast<unsigned int>(DASH_STATES::IDLE);
        }
    }
}
