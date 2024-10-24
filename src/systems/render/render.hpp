#pragma once

#include "shader.hpp"
#include "text.hpp"
#include "texture.hpp"
#include "util.hpp"

#include <array>

#include "../../common.hpp"
#include "../../components.hpp"
#include "../../ecs/ecs.hpp"

enum class GEOMETRY_BUFFER {
    SPRITE,
    SCREEN_TRIANGLE,
    GEOMETRY_COUNT,
};
constexpr int geometry_count = (int)GEOMETRY_BUFFER::GEOMETRY_COUNT;

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
    TextureManager texture_manager;
    ShaderManager shader_manager;
    TextSystem text;
    GLuint base_vao;

    mat3 projection = createProjectionMatrix();

    std::array<GLuint, geometry_count> vertex_buffers = {};
    std::array<GLuint, geometry_count> index_buffers = {};

  public:
    // Initialize the window
    bool init(GLFWwindow* window);

    template <class T>
    void bindVBOAndIBO(GEOMETRY_BUFFER gid, std::vector<T> vertices,
                       std::vector<uint16_t> indices);

    void initializeGlTextures();

    void initializeGlEffects();

    void initializeGlGeometryBuffers();
    // Initialize the screen texture used as intermediate render target
    // The draw loop first renders to this texture, then it is used for the wind
    // shader
    bool initScreenTexture();

    void initializeText();

    // Destroy resources associated to one or all entities created by the system
    ~RenderSystem();

    // Draw all entities
    void draw();

  private:
    // Internal drawing functions for each entity type
    void activeTexturedShader(Entity entity, const std::string& texture, GLuint program) const;
    void drawTexturedMesh(Entity entity) const;
    void drawToScreen() const;
    void drawText();

    // Window handle
    GLFWwindow* window = nullptr;

    // Screen texture handles
    GLuint frame_buffer = 0;
    GLuint off_screen_render_buffer_color = 0;
    GLuint off_screen_render_buffer_depth = 0;

    vec3 ambientLightColour = 0.7f * vec3(156, 194, 255);

    Entity screen_state_entity;
};

bool loadShader(const std::string& vs_path, const std::string& fs_path,
                GLuint& out_program);
