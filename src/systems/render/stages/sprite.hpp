#pragma once
#include "common.hpp"
#include "components.hpp"
#include "shader.hpp"
#include "util.hpp"

class SpriteStage {
    /**
     * Intermediate frame texture. All drawing for this stage will be output to this texture
     */
    GLuint frame_texture = 0;
    GLuint frame_buffer = 0;

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    ShaderHandle shader = 0;

    mat3 projection_matrix = createProjectionMatrix();

    float animation_speed = 200.f;

    /**
     * Vertex data for a textured quad. Each vertex contains position and UV coordinates
     */
    const TexturedVertex textured_vertices[4] = {
        {{-1.f / 2, +1.f / 2, 0.f}, {0.f, 1.f}},
        {{+1.f / 2, +1.f / 2, 0.f}, {1.f, 1.f}},
        {{+1.f / 2, -1.f / 2, 0.f}, {1.f, 0.f}},
        {{-1.f / 2, -1.f / 2, 0.f}, {0.f, 0.f}},
    };

    const uint16_t textured_indices[6] = {0, 3, 1, 1, 3, 2};

    vec3 ambient_light_colour = 0.7f * vec3(156, 194, 255);

    void createVertexAndIndexBuffers();

    void prepareDraw() const;

    void activateShader(Entity entity, const std::string& texture) const;

    void drawSprite(Entity entity, float elapsed_ms);

  public:
    void init();

    /**
     * Draw all renderable sprites onto the screen.
     */
    void draw(float elapsed_ms);

    ~SpriteStage();
};