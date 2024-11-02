#pragma once
#include "common.hpp"
#include "shader.hpp"

/**
 * Composites all other frames into one final frame.
 *
 * This pipeline stage is special as it is responsible for producing
 * the final frame that will be displayed. This stage should always be initialized and drawn last.
 */
class CompositorStage {
    // vertex data for the screen triangle (triangle that covers the entire screen)
    const vec3 screen_vertices[3] = {{-1, -6, 0.f}, {6, -1, 0.f}, {-1, 6, 0.f}};

    // Counterclockwise as it's the default opengl front winding direction.
    const uint16_t screen_indices[3] = {0, 1, 2};

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    GLuint sprite_stage_texture = 0;
    GLuint particle_stage_texture = 0;
    GLuint text_stage_texture = 0;

    ShaderHandle compositor_shader = 0;

    GLFWwindow* window = nullptr;

    void createVertexAndIndexBuffers();

    void setupTextures() const;

  public:
    /**
     * Initialize the state for the compositing stage.
     */
    void init(GLFWwindow* window_arg);

    /**
     * Composite all frames from this pipeline iteration into a single frame and apply scaling.
     */
    void draw() const;

    ~CompositorStage();
};
