// internal
#include "render.hpp"
#include "common.hpp"
#include <SDL.h>

#include "ecs/registry.hpp"

#include <iostream>

TextureManager texture_manager;
ShaderManager shader_manager;

void RenderSystem::init(GLFWwindow* window_arg) {
    this->window = window_arg;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // Load OpenGL function pointers
    const int is_fine = gl3w_init();
    assert(is_fine == 0);

    LOG_INFO("Native Resolution = {}, {}", native_width, native_height);
    LOG_INFO("Window Dimensions = {}, {}", window_width_px, window_height_px);

    texture_manager.init();
    shader_manager.init();

    sprite_stage.init();
    text_stage.init();
    composite_stage.init(window);

    checkGlErrors();
}

/**
 * Render our game world
 * http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
 */
void RenderSystem::draw() {
    sprite_stage.draw();
    text_stage.draw();
    composite_stage.draw();

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    checkGlErrors();
}