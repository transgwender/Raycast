#define GL3W_IMPLEMENTATION
#include "logging/log.hpp"
#include "logging/log_manager.hpp"
#include "particles.hpp"
#include "systems/ai.hpp"
#include "systems/physics.hpp"
#include "systems/render/render.hpp"
#include "systems/world.hpp"
#include "systems/ai.hpp"
#include "utils.h"
#include <chrono>
#include <gl3w.h>
#include <iostream>

using Clock = std::chrono::high_resolution_clock;

#define FIXED_UPDATE_MS 2

int main() {
    // Global systems
    WorldSystem world;
    RenderSystem renderer;
    PhysicsSystem physics;
    AISystem ai;
    ParticleSystem particles;

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

    // Initialize the main systems
    renderer.init(window);
    world.init();
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

        float elapsed_remainder_ms = elapsed_ms + remainder;

        world.step(elapsed_ms);
        for (int i = 0; i < (int)floor(elapsed_remainder_ms/FIXED_UPDATE_MS); ++i) {
            physics.step(FIXED_UPDATE_MS);
            world.handle_collisions();
            ai.step(FIXED_UPDATE_MS);
        }
        remainder = fmod(elapsed_remainder_ms,(float) FIXED_UPDATE_MS);
        particles.step(elapsed_ms);
        renderer.draw(elapsed_ms);
    }

    return EXIT_SUCCESS;
}
