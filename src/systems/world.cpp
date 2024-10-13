#include "world.hpp"
#include "world_init.hpp"

#include <cassert>
#include <sstream>
#include <fstream>
#include <iostream>
#include <SDL.h>

#include "components_json.hpp"
#include "systems/physics.hpp"
#include "logging/log.hpp"

// create the light-maze world
WorldSystem::WorldSystem(): next_light_spawn(0.f) {
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
    // Destroy music components
    if (background_music != nullptr)
        Mix_FreeMusic(background_music);

    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();

    // Close the window
    glfwDestroyWindow(window);
}

// Debugging
namespace {
void glfw_err_cb(int error, const char* desc) {
    LOG_ERROR("{}: {}", error, desc);
}
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
    window = glfwCreateWindow(window_width_px, window_height_px, "Raycast",
                              nullptr, nullptr);
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
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))
            ->on_mouse_button(_0, _1, _2, xpos, ypos);
    };
    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);

    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        LOG_ERROR("Failed to initialize SDL Audio");
        return nullptr;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        LOG_ERROR("Failed to open audio device");
        return nullptr;
    }

    background_music = Mix_LoadMUS(audio_path("music.wav").c_str());

    if (background_music == nullptr) {
        LOG_ERROR("Failed to load sounds. {} make sure the data "
                  "directory is present",
                  audio_path("music.wav").c_str());
        return nullptr;
    }

    return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, SceneSystem* scene_arg) {
    Scene& scene = registry.scenes.emplace(scene_state_entity);
    scene.scene_tag = "mainmenu";
    this->renderer = renderer_arg;
    this->scenes = scene_arg;
    // Playing background music indefinitely
    Mix_PlayMusic(background_music, -1);
    LOG_INFO("Loaded music");

    // Set all states to default
    restart_game();

    // Calculate the endpoints for all entities on rails
    auto& entities_on_linear_rails = registry.entitiesOnLinearRails.entities;
    LOG_INFO("Calculating endpoints for the {} entities on rails.",
             entities_on_linear_rails.size());
    for (auto e : entities_on_linear_rails) {
        Motion& e_motion = registry.motions.get(e);
        OnLinearRails& e_rails = registry.entitiesOnLinearRails.get(e);
        LinearlyInterpolatable& e_lr = registry.linearlyInterpolatables.get(e);

        auto direction = vec2(cos(e_rails.angle), sin(e_rails.angle));
        vec2 firstEndpoint = e_motion.position + e_rails.length * direction;
        vec2 secondEndpoint = e_motion.position - e_rails.length * direction;

        e_lr.t = 0.5;
        e_rails.firstEndpoint = firstEndpoint;
        e_rails.secondEndpoint = secondEndpoint;
        e_rails.direction = direction;
        LOG_INFO("Entity with position: ({}, {}) on a linear rail has "
                 "endpoints: ({},{}) ({}, {}) with a direction: ({},{})",
                 e_motion.position.x, e_motion.position.y, firstEndpoint.x,
                 firstEndpoint.y, secondEndpoint.x, secondEndpoint.y,
                 direction.x, direction.y);
        // Finally, update the angle of the entity to make sure it is aligned
        // with the rail.
        e_motion.angle = e_rails.angle;
    }
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
    float speed = 280;

    if (!registry.levels.components.empty()) {
        next_light_spawn -= elapsed_ms_since_last_update * current_speed;
        for (auto& light : registry.lightRays.components) {
            if (light.last_reflected_timeout > 0)
                light.last_reflected_timeout -= elapsed_ms_since_last_update;
        }

        if (registry.lightRays.components.size() <= MAX_LIGHT_ON_SCREEN && next_light_spawn < 0.f) {
            // reset timer
            next_light_spawn = LIGHT_SPAWN_DELAY_MS;

            auto& sources = registry.lightSources.entities;
            
            for (int i = 0; i < sources.size(); i++) {
                Zone& zone = registry.zones.get(sources[i]);
                vec2 position = zone.position;
                float angle = registry.lightSources.components[i].angle;

                const auto entity = Entity();
                createLight(entity, renderer, position,
                            vec2(cos(-angle * M_PI / 180) * speed,
                                 sin(-angle * M_PI / 180) * speed));
            }
        }
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
        scenes->try_parse_scene(registry.scenes.get(scene_state_entity).scene_tag);
    } else {
        LOG_ERROR("Hmm, there should have been a scene state entity defined.");
    }

    // Why list? please delete if not needed
    // registry.list_all_components();
}

void WorldSystem::change_scene(std::string &scene_tag) {
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
            registry.lightRays.has(collisionsRegistry.entities[i])) continue;
        if (registry.reflectives.has(collisionsRegistry.entities[i])) {
            handle_reflection(collisionsRegistry.entities[i],
                collisionsRegistry.components[i].other);
        } else {
            handle_non_reflection(collisionsRegistry.entities[i],
                collisionsRegistry.components[i].other);
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
            std::cout << "Level beaten!";
            std::string next_scene = "gamefinish";
            change_scene(next_scene);
            break;
        }
        case ZONE_TYPE::START: {
            return;
        }
        default: {
            std::cout << "Hit non-reflective object. Light ray fizzles out";
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

    Motion& light_motion = registry.motions.get(reflected);
    Motion& reflective_surface_motion = registry.motions.get(reflective);
    vec2 reflective_surface_normal = {cos(reflective_surface_motion.angle + M_PI_2),
                                        sin(reflective_surface_motion.angle + M_PI_2)};
    float angle_between = atan2(reflective_surface_normal.y,
                                reflective_surface_normal.x)
                        - atan2(light_motion.velocity.y,
                                light_motion.velocity.x);
    std::cout << angle_between * 180.f / M_PI << std::endl;
    vec2 reflected_velocity = -light_motion.velocity
        + 2.f * dot(light_motion.velocity, reflective_surface_normal) * reflective_surface_normal;
    light_motion.velocity = reflected_velocity;
    light.last_reflected = reflective;
    light.last_reflected_timeout = DOUBLE_REFLECTION_TIMEOUT;
    angle_between = atan2(reflective_surface_normal.y, reflective_surface_normal.x) - atan2(light_motion.velocity.y, light_motion.velocity.x);
    light_motion.angle -= 2 * angle_between;
    std::cout << angle_between * 180.f / M_PI << std::endl;
}

// Should the game be over?
bool WorldSystem::is_over() const {
    return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // Resetting game
    if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        restart_game();
    }

    // Control the current speed with `<` `>`
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
        key == GLFW_KEY_COMMA) {
        current_speed -= 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
        key == GLFW_KEY_PERIOD) {
        current_speed += 0.1f;
        LOG_INFO("Current speed = {}", current_speed);
    }
    current_speed = fmax(0.f, current_speed);
}

void move_mirror(vec2 position) {
    registry.motions.get(registry.reflectives.entities[0]).position = position;
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {}

void WorldSystem::on_mouse_button(int key, int action, int mod, double xpos,
                                  double ypos) {
    if (action == GLFW_RELEASE && key == GLFW_MOUSE_BUTTON_LEFT) {
        LOG_INFO("({}, {})", xpos, ypos);
        for (Entity entity : registry.interactables.entities) {
            if (registry.boundingBoxes.has(entity)) {
                BoundingBox& boundingBox = registry.boundingBoxes.get(entity);
                float xRight = boundingBox.position.x + boundingBox.scale.x / 2;
                float xLeft = boundingBox.position.x - boundingBox.scale.x / 2;
                float yUp = boundingBox.position.y - boundingBox.scale.y / 2;
                float yDown = boundingBox.position.y + boundingBox.scale.y / 2;
                if (xpos < xRight && xpos > xLeft && ypos < yDown &&
                    ypos > yUp) {

                    if (registry.changeScenes.has(entity)) {
                        ChangeScene& changeScene = registry.changeScenes.get(entity);
                        change_scene(changeScene.scene);
                    } else if (registry.rotateables.has(entity)) {
                        // Rotate the entity.
                        LOG_INFO("Something should be rotating.")
                        Motion& e_motion = registry.motions.get(entity);

                        // TODO: use lerp too smoothly rotate
                        e_motion.angle += 5 * (M_PI / 180);
                    }
                }
            }
        }
    }
}
