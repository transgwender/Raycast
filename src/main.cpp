#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <chrono>
#include "systems/physics.hpp"
#include "systems/render.hpp"
#include "systems/world.hpp"
#include "logging/log_manager.hpp"
#include "logging/log.hpp"

using Clock = std::chrono::high_resolution_clock;

int main() {
    // Global systems
    WorldSystem world;
    RenderSystem renderer;
    PhysicsSystem physics;

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

    // initialize the main systems
    renderer.init(window);
    world.init(&renderer);

    // variable time step loop
    auto t = Clock::now();
    while (!world.is_over()) {
        // Processes system messages, if this wasn't present the window would
        // become unresponsive
        glfwPollEvents();

        // Calculating elapsed times in milliseconds from the previous iteration
        auto now = Clock::now();
        float elapsed_ms =
            static_cast<float>(
                (std::chrono::duration_cast<std::chrono::microseconds>(now - t))
                    .count()) /
            1000;
        t = now;

        world.step(elapsed_ms);
        physics.step(elapsed_ms);
        world.handle_collisions();

        renderer.draw();
    }

    return EXIT_SUCCESS;
}
