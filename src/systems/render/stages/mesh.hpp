#pragma once
#include "common.hpp"
#include "components.hpp"
#include "shader.hpp"
#include "util.hpp"

class MeshStage {
    /**
     * Intermediate frame texture. All drawing for this stage will be output to this texture
     */
    TextureHandle frame_texture = 0;
    GLuint frame_buffer = 0;
    ShaderHandle shader = 0;

    mat3 projection_matrix = createProjectionMatrix();

    std::unordered_map<unsigned int, GLuint> vbos;
    std::unordered_map<unsigned int, GLuint> ibos;
    std::unordered_map<unsigned int, GLuint> vaos;

    void addMesh(const Entity& entity);

    void prepareDraw() const;

    void activateMesh(const Entity& mesh_entity);

    void activateShader(const Entity& entity);

    void drawMesh(const Entity& entity);

  public:
    void createFrame();
    void init();

    /**
     * Draw all renderable sprites onto the screen.
     */
    void draw();

    void updateShaders();

    ~MeshStage();
};