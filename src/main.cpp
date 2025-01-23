#include "animation.hpp"
#include "common.hpp"
#include "logging/log.hpp"
#include "logging/log_manager.hpp"
#include "particles.hpp"
#include "systems/ai.hpp"
#include "systems/physics.hpp"
#include "systems/render/render.hpp"
#include "systems/world.hpp"
#include <chrono>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#endif

using Clock = std::chrono::high_resolution_clock;

#define FIXED_UPDATE_MS 2

bool window_focused = true;

GLFWwindow* window;

float remainder_time = 0;
std::chrono::time_point<std::chrono::steady_clock> last_time;

// Global systems
WorldSystem world;
RenderSystem renderer;
PhysicsSystem physics;
AISystem ai;
AnimationSystem animation;
ParticleSystem particles;
PersistenceSystem persistence;

/**
 * Advance the game loop one step
 */
void step_game_loop() {
    if (world.is_over()) {
        persistence.try_write_save();
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#else
        exit(0);
#endif
    }

    // Processes system messages, if this wasn't present the window would
    // become unresponsive
    glfwPollEvents();
    // Calculating elapsed times in milliseconds from the previous iteration
    auto now = Clock::now();
    float elapsed_ms =
        static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - last_time)).count()) / 1000;
    last_time = now;

    if (window_focused) {
        float elapsed_remainder_ms = elapsed_ms + remainder;
        world.step(elapsed_ms);
        for (int i = 0; i < (int)floor(elapsed_remainder_ms / FIXED_UPDATE_MS); ++i) {
            physics.step(FIXED_UPDATE_MS);
            physics.detect_collisions();
            world.handle_collisions();
        }
        ai.step(elapsed_ms);
        animation.step(elapsed_ms);
        remainder = fmod(elapsed_remainder_ms, (float)FIXED_UPDATE_MS);
        particles.step(elapsed_ms);
        renderer.draw(elapsed_ms);
    }
}

int main() {
    // Initialize default logger
    raycast::logging::LogManager log_manager;
    log_manager.Initialize();

    // Initializing window
    window = world.create_window();
    if (!window) {
        LOG_ERROR("Failed to initialize GLFW window");
        return EXIT_FAILURE;
    }

    // Define window focus callback inside main
    auto on_window_focus = [](GLFWwindow* wnd, int focused) {
        if (focused) {
            window_focused = true;
            LOG_INFO("Window in focus.");
        } else {
            window_focused = false;
            LOG_INFO("Window out of focus.");
        }
    };

    glfwSetWindowFocusCallback(window, on_window_focus);

    // Initialize the main systems
    renderer.init(window);
    persistence.init();
    world.init(&persistence);
    particles.init();

    // Variable time step loop
    last_time = Clock::now();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(step_game_loop, 0, 1);
#else
    while (true) {
        step_game_loop();
    }
#endif

    return EXIT_SUCCESS;
}
