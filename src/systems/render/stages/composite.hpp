#pragma once
#include "common.hpp"
#include "shader.hpp"

class CompositeStage {
    // vertex data for the screen triangle (triangle that covers the entire screen)
    const vec3 screen_vertices[3] = {{-1, -6, 0.f}, {6, -1, 0.f}, {-1, 6, 0.f}};

    // Counterclockwise as it's the default opengl front winding direction.
    const uint16_t screen_indices[3] = {0, 1, 2};

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    ShaderHandle screen_shader = 0;

    GLFWwindow* window = nullptr;

    void createVertexAndIndexBuffers();

  public:
    /**
     * Initialize the state for the compositing stage.
     */
    void init(GLFWwindow* window_arg);

    /**
     * Composite all frames from this pipeline iteration into a single frame and apply scaling.
     * @param frame_texture OpenGL texture id of the texture that the final frame was rendered to
     */
    void draw(GLuint frame_texture) const;

    ~CompositeStage();
};
