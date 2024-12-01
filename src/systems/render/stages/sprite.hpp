#pragma once
#include "common.hpp"
#include "components.hpp"
#include "shader.hpp"
#include "util.hpp"

/**
* Render all sprites in the world excluding text.
*/
class SpriteStage {
    /**
     * Intermediate frame texture. All drawing for this stage will be output to this texture
     */
    TextureHandle world_texture = 0;
    TextureHandle ui_texture = 0;
    GLuint frame_buffer = 0;

    ShaderHandle shader = 0;

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    mat3 projection_matrix = createProjectionMatrix();

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

    vec3 default_ambient_light_colour = 1.0f * vec3(156, 194, 255);

    void createVertexAndIndexBuffers();

    void prepareDraw() const;

    void activateShader(const Entity& entity, const Motion& motion, const Material& material) const;

    void drawSprite(const Entity& entity, const Material& material) const;

  public:
    void init();

    /**
     * Draw all renderable sprites onto the screen.
     */
    void draw() const;

    void updateShaders();

    ~SpriteStage();
};