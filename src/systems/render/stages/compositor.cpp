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

void CompositorStage::init(GLFWwindow* window_arg) {
    window = window_arg;
    createVertexAndIndexBuffers();

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
        const auto location = glGetUniformLocation(shader, texture_names[i]);
        glUniform1i(location, i);

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
}

void CompositorStage::draw() const {
    glUseProgram(shader);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthRange(0, 10);
    glClearColor(0.f, 0, 0, 0.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // Draw the screen texture on the quad geometry
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Set clock
    setUniformFloat(shader, "time", static_cast<float>(glfwGetTime() * 10.0f));

    // Set the vertex position and vertex texture coordinates (both stored in the same VBO)
    GLint position_location = glGetAttribLocation(shader, "in_position");
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)nullptr);

    setupTextures();

    // Draw
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();

    glBindVertexArray(0);
    glUseProgram(0);
}

void CompositorStage::updateShaders() {
    shader = shader_manager.get("compositor");
}

CompositorStage::~CompositorStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    checkGlErrors();
}
