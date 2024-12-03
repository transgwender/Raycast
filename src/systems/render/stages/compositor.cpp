#include "compositor.hpp"

#include "render.hpp"

void CompositorStage::createVertexAndIndexBuffers() {
    // vertex and index buffer for the screen triangle
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices), screen_vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screen_indices), screen_indices,
                 GL_STATIC_DRAW);

    checkGlErrors();
}

void CompositorStage::createTextures() {
    // create new render textures and bind it to our new framebuffer
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glGenTextures(1, &composited_texture);
    glBindTexture(GL_TEXTURE_2D, composited_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, upscaled_width, upscaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, composited_texture, 0);

    glGenFramebuffers(1, &bloom_buffer0);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer0);
    glGenTextures(1, &bloom_tex0);
    glBindTexture(GL_TEXTURE_2D, bloom_tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bloom_pass_width, bloom_pass_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex0, 0);

    glGenFramebuffers(1, &bloom_buffer1);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer1);
    glGenTextures(1, &bloom_tex1);
    glBindTexture(GL_TEXTURE_2D, bloom_tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bloom_pass_width, bloom_pass_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex1, 0);
}

void CompositorStage::init(GLFWwindow* window_arg) {
    window = window_arg;

    // get a reference to the frame textures from the other pipeline stages.
    world_texture = texture_manager.get("$world_texture");
    ui_texture = texture_manager.get("$ui_texture");
    world_text_texture = texture_manager.get("$world_text");
    ui_text_texture = texture_manager.get("$ui_text");

    createVertexAndIndexBuffers();
    createTextures();

    updateShaders();

    // create vao
    glGenVertexArrays(1, &vao);
}

void CompositorStage::setupTextures() const {
    static const char* texture_names[] = {
        "world",
        "ui",
        "world_text",
        "ui_text",
    };

    static GLuint textures[] = {
        bloom_tex1,
        ui_texture,
        world_text_texture,
        ui_text_texture
    };

    for (int i = 0; i < std::size(texture_names); i++) {
        const auto location = glGetUniformLocation(compositor_shader, texture_names[i]);
        glUniform1i(location, i);

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
}

void CompositorStage::prepare() const {
    glDepthRange(0, 10);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0);
    glClearDepth(1.f);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    checkGlErrors();
}

void CompositorStage::composite() const {
    glUseProgram(compositor_shader);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set the vertex position and vertex texture coordinates (both stored in the same VBO)
    GLint position_location = glGetAttribLocation(compositor_shader, "in_position");
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)nullptr);

    setupTextures();

    // Draw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();
}

void CompositorStage::bloomBrightnessPass() const {
    glViewport(0, 0, bloom_pass_width, bloom_pass_height);

    glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, world_texture);
    setUniformInt(post_processor_shader, "mode", 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
}

void CompositorStage::bloomBlurPass() const {
    constexpr int blur_passes = 4;

    glViewport(0, 0, bloom_pass_width, bloom_pass_height);
    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < blur_passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, i % 2 == 0 ? bloom_buffer1 : bloom_buffer0);
        glBindTexture(GL_TEXTURE_2D, i % 2 == 0 ? bloom_tex0 : bloom_tex1);
        setUniformInt(post_processor_shader, "mode", (i % 2) + 1);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    }
}

void CompositorStage::bloomComposite() const {

    glViewport(0, 0, bloom_pass_width, bloom_pass_height);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer1);

    setUniformInt(post_processor_shader, "input1", 0);
    setUniformInt(post_processor_shader, "input2", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bloom_tex0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, world_texture);

    setUniformInt(post_processor_shader, "mode", 3);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
}

void CompositorStage::postProcess() const {
    glUseProgram(post_processor_shader);

    setUniformFloatVec2(post_processor_shader, "screen_size", vec2(upscaled_width, upscaled_height));

    bloomBrightnessPass();
    bloomBlurPass();
    bloomComposite();

    checkGlErrors();
}

void CompositorStage::draw() const {
    prepare();
    postProcess();
    composite();
}

void CompositorStage::updateShaders() {
    compositor_shader = shader_manager.get("compositor");
    post_processor_shader = shader_manager.get("post_processor");
}

CompositorStage::~CompositorStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    checkGlErrors();
}
