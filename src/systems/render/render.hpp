#pragma once

#include "common.hpp"
#include "compositor.hpp"
#include "mesh.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "stages/sprite.hpp"
#include "stages/text.hpp"
#include "texture.hpp"

/**
 * Global texture manager. There should only be one instance of this.
 */
extern TextureManager texture_manager;

/**
 * Global shader manager. There should only be one instance of this.
 */
extern ShaderManager shader_manager;

/**
 * Shorthand for getting a texture from the texture manager.
 * @param name Name of the texture
 * @return A texture handle for the texture with the given `name`
 */
inline TextureMaterial get_tex(const std::string& name) {
    return {name, texture_manager.get(name), texture_manager.getNormal(name)};
}

/**
 * System responsible for setting up OpenGL and for rendering all the
 * visual entities in the game.
 */
class RenderSystem {
    SpriteStage world_stage;
    MeshStage mesh_stage;
    ParticleStage particle_stage;
    TextStage text_stage;
    CompositorStage composite_stage;

    /** Window handle */
    GLFWwindow* window = nullptr;

    float hot_reload_interval = 1000.0;
    float hot_reload_time = 0.0;

    void updateShaders();

    void updateTextures();

  public:
    /** Initialize the window. */
    void init(GLFWwindow* window);

    /** Draw all visible, renderable entities. */
    void draw(float elapsed_ms);
};
