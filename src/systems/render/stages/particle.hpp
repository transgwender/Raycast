#pragma once

#include "common.hpp"
#include "components.hpp"
#include "shader.hpp"
#include "util.hpp"

/**
 * Data for a single particle that will be sent to the GPU.
 * Must follow OpenGL's alignment requirements.
 */
#pragma pack(push, 1)
struct ParticleGPUData {
    vec2 position;
    vec2 scale;
    vec4 color;
    float angle;
    float _padding[3] = {0, 0, 0};
};
#pragma pack(pop)

/**
 * Renders particles into the world.
 *
 * This stage uses an instanced renderer and batches draw calls
 * by particle groups (basically particles that all use the same texture).
 *
 * This stage has two buffers. One is a buffer that stores information for a generic
 * textured quad, which is static across all instances. The other buffer stores information
 * specific to each instance, such as position, scale, angle, and color. This buffer
 * can be updated every frame as particle information changes. The transform
 * matrix is constructed in the vertex shader and applied to the generic quad's object coordinates.
 * The renderer then groups the particles by their texture, and does one instanced draw call for every group.
 *
 * "Improving Learn OpenGL's Text Rendering Example" by Whatever's Right Studios,
 * the Batch Rendering series by The Cherno, and LearnOpenGL's instanced rendering page
 * were good resources for learning more about instanced rendering.
 * Ultimately I went in a different direction than those resources, though they were still helpful.
 */
class ParticleStage {
    GLuint frame_buffer = 0;
    TextureHandle frame_texture = 0;

    ShaderHandle shader = 0;

    GLuint quad_vbo = 0;
    GLuint instance_vbo = 0;
    GLuint vao = 0;

    GLsizei instance_buffer_size = 100;

    mat3 projection_matrix = createProjectionMatrix();

    /**
     * Vertex data for a textured quad. Each vertex contains position and UV coordinates
     */
    const GLfloat textured_vertices[4][4] = {
        {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}};

    void createBuffers();

    void allocateInstanceBuffer() const;

    void initVAO();

    void prepareDraw() const;

  public:
    void init();

    void draw();

    void updateShaders();

    ~ParticleStage();
};
