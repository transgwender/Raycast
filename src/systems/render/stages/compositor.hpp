#pragma once
#include "common.hpp"
#include "shader.hpp"
#include "texture.hpp"

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

    GLuint frame_buffer = 0;
    GLuint bloom_buffer0 = 0;
    GLuint bloom_buffer1 = 0;

    TextureHandle world_texture = 0;
    TextureHandle ui_texture = 0;
    TextureHandle world_text_texture = 0;
    TextureHandle ui_text_texture = 0;

    TextureHandle composited_texture = 0;

    TextureHandle bloom_tex0 = 0;
    TextureHandle bloom_tex1 = 0;

    ShaderHandle compositor_shader = 0;
    ShaderHandle post_processor_shader = 0;

    int bloom_pass_width = native_width * 2;
    int bloom_pass_height = native_height * 2;

    GLFWwindow* window = nullptr;

    void createVertexAndIndexBuffers();
    void createTextures();

    void setupTextures() const;
    void prepare() const;

  public:
    /**
     * Initialize the state for the compositing stage.
     */
    void init(GLFWwindow* window_arg);

    /**
     * Composite all frames from this pipeline iteration into a single frame and apply scaling.
     */
    void composite() const;
    void bloomBrightnessPass() const;
    void bloomBlurPass() const;
    void bloomComposite() const;
    void postProcess() const;
    void draw() const;

    void updateShaders();

    ~CompositorStage();
};
