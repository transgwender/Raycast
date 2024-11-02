#pragma once

#include "common.hpp"
#include "compositor.hpp"
#include "shader.hpp"
#include "sprite.hpp"
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
 * System responsible for setting up OpenGL and for rendering all the
 * visual entities in the game.
 */
class RenderSystem {
    SpriteStage sprite_stage;
    CompositorStage composite_stage;
    TextStage text_stage;

    /** Window handle */
    GLFWwindow* window = nullptr;

  public:
    /** Initialize the window. */
    void init(GLFWwindow* window);

    /** Draw all visible, renderable entities. */
    void draw(float elapsed_ms);
};
