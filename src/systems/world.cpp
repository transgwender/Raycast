#include "world.hpp"
#include "world_init.hpp"

#include <SDL.h>
#include <cassert>
#include <fstream>
#include <iostream>

#include "components_json.hpp"
#include "logging/log.hpp"
#include "systems/menu.hpp"
#include "systems/physics.hpp"
#include "systems/rails.hpp"

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

void WorldSystem::init() {
    scenes.init(scene_state_entity);
    sounds.load_all_sounds();

    // Set all states to default
    restart_game();
}

bool WorldSystem::isInLevel() {
    assert(registry.levels.size() <= 1);
    return !registry.levels.components.empty();
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
    float speed = 100;
    if (isInLevel() && shouldStep()) {
        next_light_spawn -= elapsed_ms_since_last_update * current_speed;

        for (int i = 0; i < registry.lightRays.components.size(); i++) {
            auto& lightEntity = registry.lightRays.entities[i];
            auto& light = registry.lightRays.components[i];
            if (light.last_reflected_timeout > 0)
                light.last_reflected_timeout -= elapsed_ms_since_last_update;
            Motion& motion = registry.motions.get(lightEntity);
            if (motion.position.x < -15 || motion.position.x > native_width + 15 || motion.position.y < -15 ||
                motion.position.y > native_height + 15) {
                registry.remove_all_components_of(lightEntity);
            }
        }

        if (registry.lightRays.components.size() < MAX_LIGHT_ON_SCREEN && next_light_spawn < 0.f) {
            // reset timer
            next_light_spawn = LIGHT_SPAWN_DELAY_MS;

            auto& sources = registry.lightSources.entities;

            for (int i = 0; i < sources.size(); i++) {
                Zone& zone = registry.zones.get(sources[i]);
                vec2 position = zone.position;
                float angle = registry.lightSources.components[i].angle;

                const auto entity = Entity();
                createLight(entity, position, vec2(cos(-angle * M_PI / 180) * speed, sin(-angle * M_PI / 180) * speed));
            }
        }

        rails.step(elapsed_ms_since_last_update);
        updateDash();
    }

    return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
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

    if(!registry.levelSelects.components.empty()) {
        menus.generate_level_select_buttons((int) scenes.level_count());
    }

    rails.init(); // TODO: It feels weird having an init in a reset. Maybe change this to be reset?
}

void WorldSystem::change_scene(std::string& scene_tag) {
    Scene& scene = registry.scenes.get(scene_state_entity);
    scene.scene_tag = scene_tag;
    restart_game(); // TODO: Change to function for changing scene specifically
}

// Handle collisions between entities
void WorldSystem::handle_collisions() {
    // registry.collisions.emplace_with_duplicates(Entity(), Entity());
    // Loop over all collisions detected by the physics system
    auto& collisionsRegistry = registry.collisions;
    for (int i = 0; i < collisionsRegistry.size(); i++) {
        // for now, only handle collisions involving light ray as other object
        if (!registry.lightRays.has(collisionsRegistry.components[i].other) ||
            registry.lightRays.has(collisionsRegistry.entities[i]))
            continue;
        if (registry.reflectives.has(collisionsRegistry.entities[i])) {
            handle_reflection(collisionsRegistry.entities[i], collisionsRegistry.components[i].other);
        } else {
            handle_non_reflection(collisionsRegistry.entities[i], collisionsRegistry.components[i].other);
        }
    }
    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

// When colliding entities to not reflect, if one of the entities
// is a light ray and the other is an end zone, win the level
// if the other entity is not an end-zone, destroy light ray
// Invariant: other is a light ray
void WorldSystem::handle_non_reflection(Entity& collider, Entity& other) {
    assert(registry.lightRays.has(other));
    if (registry.zones.has(collider))
        switch (registry.zones.get(collider).type) {
        case ZONE_TYPE::END: {
            LOG_INFO("Level beaten!");
//            std::string next_scene = "gamefinish";
//            change_scene(next_scene);
            assert(registry.levels.size() == 1);
            Level& level = registry.levels.components.front();
            if (registry.menus.components.empty()) {
                menus.generate_level_win_popup(level.id, (int) scenes.level_count());
            }
//            registry.remove_all_components_of(other);
            break;
        }
        case ZONE_TYPE::START: {
            return;
        }
        default: {
            sounds.play_sound("light-collision.wav");
            LOG_INFO("Hit non-reflective object. Light ray fizzles out");
            registry.remove_all_components_of(other);
            break;
        }
        }
}

// Reflect light ray based on collision normal
// Invariant: other is a light ray
void WorldSystem::handle_reflection(Entity& reflective, Entity& reflected) {
    assert(registry.lightRays.has(reflected));
    Light& light = registry.lightRays.get(reflected);
    // don't reflect off of same mirror twice
    // due to being inside
    if (light.last_reflected == reflective && light.last_reflected_timeout > 0) {
        return;
    }

    sounds.play_sound("light-collision.wav");

    Motion& light_motion = registry.motions.get(reflected);
    Motion& reflective_surface_motion = registry.motions.get(reflective);
    vec2 reflective_surface_normal = {cos(reflective_surface_motion.angle + M_PI_2),
                                      sin(reflective_surface_motion.angle + M_PI_2)};
    float angle_between = atan2(reflective_surface_normal.y, reflective_surface_normal.x) -
                          atan2(light_motion.velocity.y, light_motion.velocity.x);
    std::cout << angle_between * 180.f / M_PI << std::endl;
    vec2 reflected_velocity = -light_motion.velocity +
                              2.f * dot(light_motion.velocity, reflective_surface_normal) * reflective_surface_normal;
    light_motion.velocity = reflected_velocity;
    light.last_reflected = reflective;
    light.last_reflected_timeout = DOUBLE_REFLECTION_TIMEOUT;
    angle_between = atan2(reflective_surface_normal.y, reflective_surface_normal.x) -
                    atan2(light_motion.velocity.y, light_motion.velocity.x);
    light_motion.angle -= 2 * angle_between;
    std::cout << angle_between * 180.f / M_PI << std::endl;
}

// Should the game be over?
bool WorldSystem::is_over() const { return bool(glfwWindowShouldClose(window)); }

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // Resetting game
    if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        restart_game();
    }

    // Pausing
    if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        assert(registry.menus.size() <= 1);
        if (registry.menus.size() == 0) {
            assert(registry.menus.size() <= 1);
            if (registry.levels.size() > 0) {
                Level &level = registry.levels.components.front();
                menus.generate_pause_popup(level.id);
            }
        } else {
            Menu &menu = registry.menus.components.front();
            if (menu.canClose) {
                menus.try_close_menu();
            }
        }
    }

    // Control the current speed with `<` `>`
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
        current_speed -= 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
        current_speed += 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    current_speed = fmax(0.f, current_speed);
}

void move_mirror(vec2 position) { registry.motions.get(registry.reflectives.entities[0]).position = position; }

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    vec2 world_pos = screenToWorld(mouse_position);
    for (Entity entity : registry.interactables.entities) {
        Motion& motion = registry.motions.get(entity);
        if (registry.buttons.has(entity)) {
            // Handle buttons separately — just keep piling on jank!
            if ((motion.position + abs(motion.scale / 2.f)).y > world_pos.y &&
                (motion.position + abs(motion.scale / 2.f)).x > world_pos.x &&
                (motion.position - abs(motion.scale / 2.f)).x < world_pos.x &&
                (motion.position - abs(motion.scale / 2.f)).y < world_pos.y) {
                if (registry.highlightables.has(entity)) {
                    registry.highlightables.get(entity).isHighlighted = true;
                }
                return;
            } else {
                if (registry.highlightables.has(entity)) {
                    registry.highlightables.get(entity).isHighlighted = false;
                }
            }
        }
        if (dot(world_pos - motion.position, world_pos - motion.position)
                < dot(motion.scale/2.f, motion.scale/2.f)) {
            if (registry.highlightables.has(entity)) {
                registry.highlightables.get(entity).isHighlighted = true;
            }
        } else {
            if (registry.highlightables.has(entity)) {
                registry.highlightables.get(entity).isHighlighted = false;
            }
        }
    }
}

void WorldSystem::on_mouse_button(int key, int action, int mod, double xpos, double ypos) {
    if (action == GLFW_RELEASE && key == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 world_pos = screenToWorld(vec2(xpos, ypos));
        LOG_INFO("({}, {})", xpos, ypos);
        for (Entity entity : registry.interactables.entities) {
            assert(registry.motions.has(entity));
            Motion& motion = registry.motions.get(entity);
            if (registry.buttons.has(entity)) {
                if ((motion.position + abs(motion.scale/2.f)).y > world_pos.y &&
                    (motion.position + abs(motion.scale/2.f)).x > world_pos.x &&
                    (motion.position - abs(motion.scale/2.f)).x < world_pos.x &&
                    (motion.position - abs(motion.scale/2.f)).y < world_pos.y) {
                    // Handle buttons separately — just keep piling on jank!
                    if (registry.changeScenes.has(entity)) {
                        ChangeScene& changeScene = registry.changeScenes.get(entity);
                        change_scene(changeScene.scene);
                        return;
                    } else if (registry.resumeGames.has(entity)) {
                        menus.try_close_menu();
                        return;
                    }
                }
            } else {
                if (shouldAllowInput()) {
                    if (dot(world_pos - motion.position, world_pos - motion.position)
                        < dot(motion.scale/2.f, motion.scale/2.f)) {
                        if (registry.rotateables.has(entity)) {

                            // Rotate the entity.
                            LOG_INFO("Something should be rotating.")
                            Motion& e_motion = registry.motions.get(entity);

                            // TODO: use lerp too smoothly rotate
                            float ANGLE_TO_ROTATE = 5 * (M_PI / 180);
                            e_motion.angle += motion.position.x > world_pos.x ? -ANGLE_TO_ROTATE : ANGLE_TO_ROTATE;
                        }

                        if (registry.lerpables.has(entity)) {
                            Lerpable& e_lr = registry.lerpables.get(entity);
                            e_lr.t_step = 0;
                        }
                    }
                }
            }
        }
    }
    if (action == GLFW_PRESS && key == GLFW_MOUSE_BUTTON_LEFT) {
        vec2 world_pos = screenToWorld(vec2(xpos, ypos));
        LOG_INFO("({}, {})", xpos, ypos);
        for (Entity entity : registry.interactables.entities) {
            if (shouldAllowInput()) {
                Motion& motion = registry.motions.get(entity);

                if (dot(world_pos - motion.position, world_pos - motion.position) <
                    dot(motion.scale / 2.f, motion.scale / 2.f)) {
                    if (registry.entitiesOnLinearRails.has(entity)) {
                        LOG_INFO("Moving entity on linear rail.");
                        OnLinearRails& e_rails = registry.entitiesOnLinearRails.get(entity);
                        Lerpable& e_lr = registry.lerpables.get(entity);
                        int which_direction = dot(motion.position - world_pos, e_rails.direction);
                        if (which_direction > 0) {
                            e_lr.t_step = -0.5;
                        } else if (which_direction < 0) {
                            e_lr.t_step = 0.5;
                        }
                    }
                }
            }
        }
    }
}

void WorldSystem::updateDash() {

    for (Entity dashEntity : registry.turtles.entities) {
        DASH_STATES dash_state = registry.turtles.get(dashEntity).behavior;
        vec2 ray = registry.turtles.get(dashEntity).nearestLightRayDirection;
        Motion& dm = registry.motions.get(dashEntity);

        if (registry.spriteSheets.components.empty()) {
            throw std::runtime_error("Error: Sprite sheet does not exist. Please make sure to add the sprite sheet to the level JSON.");
        }

        SpriteSheet& ss = registry.spriteSheets.components.front();

        if (dash_state == DASH_STATES::WALK) {
            // vec2 displacement = {(dm.position.x - ray.x), (dm.position.y - ray.y)};
            ss.currState = static_cast<unsigned int>(DASH_STATES::WALK);
            if (ray.x > 0) {
                dm.velocity.x = -dashSpeed;
            } else if (ray.x < 0) {
                dm.velocity.x = dashSpeed;
            }

            //if (dm.position.x < 100) {
            //    int w = 0;
            //    w++;
            //}

        } else if (dash_state == DASH_STATES::STARE) {
            dm.velocity = {0, 0};
            ss.currState = static_cast<unsigned int>(DASH_STATES::STARE);

        } else if (dash_state == DASH_STATES::IDLE) {
            dm.velocity = {0, 0};
            ss.currState = static_cast<unsigned int>(DASH_STATES::IDLE);
        }
    }
}
