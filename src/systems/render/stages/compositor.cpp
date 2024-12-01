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

void CompositorStage::createScreenTexture() {
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create new render textures and bind it to our new framebuffer
    glGenTextures(1, &composited_texture);
    glBindTexture(GL_TEXTURE_2D, composited_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, upscaled_width, upscaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, composited_texture, 0);
}

void CompositorStage::init(GLFWwindow* window_arg) {
    window = window_arg;
    createVertexAndIndexBuffers();
    createScreenTexture();

    // get a reference to the frame textures from the other pipeline stages.
    world_texture = texture_manager.get("$world_texture");
    ui_texture = texture_manager.get("$ui_texture");
    world_text_texture = texture_manager.get("$world_text");
    ui_text_texture = texture_manager.get("$ui_text");

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
        world_texture,
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

    glViewport(0, 0, upscaled_width, upscaled_height);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

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

void CompositorStage::postProcess() const {
    glUseProgram(post_processor_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, composited_texture);
    setUniformInt(post_processor_shader, "screen", 0);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();
}

void CompositorStage::draw() const {
    prepare();
    composite();
    postProcess();
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
