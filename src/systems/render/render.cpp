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
    post_processing_stage.init();
    text_stage.init();
}

RenderSystem::~RenderSystem() {
    // remove all entities created by the render system
    while (!registry.renderables.entities.empty())
        registry.remove_all_components_of(registry.renderables.entities.back());
}

/**
 * Render our game world
 * http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
 */
void RenderSystem::draw() {
    GLuint sprite_stage_result = sprite_stage.draw();
    post_processing_stage.draw(window, sprite_stage_result);
    text_stage.draw();

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    checkGlErrors();
}