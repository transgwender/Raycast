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

    world_stage.init();
    mesh_stage.init();
    particle_stage.init();
    text_stage.init();
    composite_stage.init(window);

    checkGlErrors();
}

void RenderSystem::updateShaders() {
    world_stage.updateShaders();
    mesh_stage.updateShaders();
    particle_stage.updateShaders();
    text_stage.updateShaders();
    composite_stage.updateShaders();
}

void RenderSystem::updateTextures() {
    for (auto& material : registry.materials.components) {
        material.texture.albedo = texture_manager.get(material.texture.name);
        material.texture.normal = texture_manager.getNormal(material.texture.name);
    }
}

/**
 * Render our game world
 * http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
 */
void RenderSystem::draw(float elapsed_ms) {
    hot_reload_time += elapsed_ms;
    if (hot_reload_time > hot_reload_interval) {
        hot_reload_time = 0.0;
        if (texture_manager.update()) {
            updateTextures();
        }
        if (shader_manager.update()) {
            updateShaders();
        }
    }

    world_stage.draw();
    mesh_stage.draw();
    particle_stage.draw();
    text_stage.draw();
    composite_stage.draw();

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    checkGlErrors();
}
