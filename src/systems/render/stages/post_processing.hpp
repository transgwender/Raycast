#pragma once
#include "common.hpp"
#include "shader.hpp"

class PostProcessingStage {
    // vertex data for the screen triangle (triangle that covers the entire screen)
    const vec3 screen_vertices[3] = {{-1, -6, 0.f}, {6, -1, 0.f}, {-1, 6, 0.f}};

    // Counterclockwise as it's the default opengl front winding direction.
    const uint16_t screen_indices[3] = {0, 1, 2};

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    ShaderHandle screen_shader = 0;

    void createVertexAndIndexBuffers();

  public:
    void init();

    void draw(GLFWwindow* window, GLuint frame_texture) const;

    ~PostProcessingStage();
};
