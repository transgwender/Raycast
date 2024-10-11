// Header
#include "world.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "systems/physics.hpp"

#include <fstream>
#include <iostream>


const size_t LIGHT_SPAWN_DELAY_MS = 2000 * 3;

// create the underwater world
WorldSystem::WorldSystem(): next_light_spawn(0.f) {
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

    // destroy music components
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
    fprintf(stderr, "%d: %s", error, desc);
}
} // namespace

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the
// renderer
GLFWwindow* WorldSystem::create_window() {
    ///////////////////////////////////////
    // Initialize GLFW
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW");
        return nullptr;
    }

    //-------------------------------------------------------------------------
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
        fprintf(stderr, "Failed to glfwCreateWindow");
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

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return nullptr;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return nullptr;
    }

    background_music = Mix_LoadMUS(audio_path("music.wav").c_str());

    if (background_music == nullptr) {
        fprintf(stderr,
                "Failed to load sounds\n %s make sure the data "
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
    fprintf(stderr, "Loaded music\n");

    // Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
    float speed = 80;
    if (!registry.levels.components.empty()) {
        next_light_spawn -= elapsed_ms_since_last_update * current_speed;
        if (registry.lightRays.components.size() <= 5 &&
            next_light_spawn < 0.f) {
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
    printf("Restarting\n");

    // Reset the game speed
    current_speed = 1.f;

    // Remove all entities that we created
    registry.clear_all_components();

    // Debugging for memory/component leaks
    registry.list_all_components();

    // Parse scene file
    if (registry.scenes.has(scene_state_entity)) {
        scenes->try_parse_scene(registry.scenes.get(scene_state_entity).scene_tag);
    } else {
        printf("ERROR: NO SCENE STATE ENTITY\n");
    }
}

void WorldSystem::change_scene(std::string &scene_tag) {
    Scene& scene = registry.scenes.get(scene_state_entity);
    scene.scene_tag = scene_tag;
    restart_game(); // TODO: Change to function for changing scene specifically
}

// Handle collisions between entities
void WorldSystem::handle_collisions() {}

// Should the game be over ?
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
        printf("Current speed = %f\n", current_speed);
    }
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) &&
        key == GLFW_KEY_PERIOD) {
        current_speed += 0.1f;
        printf("Current speed = %f\n", current_speed);
    }
    current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {}

void WorldSystem::on_mouse_button(int key, int action, int mod, double xpos, double ypos) {
    if (action == GLFW_RELEASE && key == GLFW_MOUSE_BUTTON_LEFT) {
        printf("(%f, %f)\n", xpos, ypos);
        for(Entity entity : registry.interactables.entities) {
            if (registry.boundingBoxes.has(entity)) {
                BoundingBox& boundingBox = registry.boundingBoxes.get(entity);
                float xRight = boundingBox.position.x + boundingBox.scale.x/2;
                float xLeft = boundingBox.position.x - boundingBox.scale.x/2;
                float yUp = boundingBox.position.y - boundingBox.scale.y/2;
                float yDown = boundingBox.position.y + boundingBox.scale.y/2;
                if (xpos < xRight && xpos > xLeft && ypos < yDown && ypos > yUp) {
                    if (registry.changeScenes.has(entity)) {
                        ChangeScene& changeScene = registry.changeScenes.get(entity);
                        change_scene(changeScene.scene);
                    }
                }
            }
        }
    }
}