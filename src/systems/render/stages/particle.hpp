#pragma once

#include "common.hpp"
#include "components.hpp"
#include "util.hpp"

class ParticleStage {
    GLuint frame_buffer = 0;
    GLuint frame_texture = 0;

    GLuint vbo = 0;
    GLuint vao = 0;

    GLuint shader = 0;

    mat3 projection_matrix = createProjectionMatrix();

    /**
     * Vertex data for a textured quad. Each vertex contains position and UV coordinates
     */
    const GLfloat textured_vertices[4][4] = {
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 1.0f}
    };

    void createVertexAndIndexBuffers();

    void prepareDraw() const;

    void activateShader(const std::string& texture) const;

  public:
    void init();

    void draw();

    ~ParticleStage();
};
