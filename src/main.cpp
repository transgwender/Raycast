#define GL3W_IMPLEMENTATION
#include "animation.hpp"
#include "logging/log.hpp"
#include "logging/log_manager.hpp"
#include "particles.hpp"
#include "systems/ai.hpp"
#include "systems/physics.hpp"
#include "systems/render/render.hpp"
#include "systems/world.hpp"
#include "utils.h"
#include <chrono>
#include <gl3w.h>
#include <iostream>

using Clock = std::chrono::high_resolution_clock;

#define FIXED_UPDATE_MS 2

bool window_focused = true;

int main() {
    // Global systems
    WorldSystem world;
    RenderSystem renderer;
    PhysicsSystem physics;
    AISystem ai;
    AnimationSystem animation;
    ParticleSystem particles;
    PersistenceSystem persistence;

    // Initialize default logger
    raycast::logging::LogManager log_manager;
    log_manager.Initialize();

    // Initializing window
    GLFWwindow* window = world.create_window();
    if (!window) {
        LOG_ERROR("Failed to initialize GLFW window");
        LOG_INFO("Press any key to exit");
        getchar();
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

    float remainder = 0;

    // Variable time step loop
    auto t = Clock::now();
    while (!world.is_over()) {
        // Processes system messages, if this wasn't present the window would
        // become unresponsive
        glfwPollEvents();
        // Calculating elapsed times in milliseconds from the previous iteration
        auto now = Clock::now();
        float elapsed_ms =
            static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count()) / 1000;
        t = now;

        if (window_focused) {
            float elapsed_remainder_ms = elapsed_ms + remainder;

            world.step(elapsed_ms);
            for (int i = 0; i < (int)floor(elapsed_remainder_ms / FIXED_UPDATE_MS); ++i) {
                physics.step(FIXED_UPDATE_MS);
            }
            physics.detect_collisions();
            world.handle_collisions();
            ai.step(elapsed_ms);
            animation.step(elapsed_ms);
            remainder = fmod(elapsed_remainder_ms, (float)FIXED_UPDATE_MS);
            particles.step(elapsed_ms);
            renderer.draw(elapsed_ms);
        }
    }

    persistence.try_write_save();

    return EXIT_SUCCESS;
}
