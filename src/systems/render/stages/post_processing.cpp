#include "post_processing.hpp"

#include "render.hpp"

void PostProcessingStage::createVertexAndIndexBuffers() {
    // vertex and index buffer for the screen triangle
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices[0]) * std::size(screen_vertices), screen_vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screen_indices[0]) * std::size(screen_indices), screen_indices,
                 GL_STATIC_DRAW);

    checkGlErrors();
}

void PostProcessingStage::init() {
    createVertexAndIndexBuffers();
    screen_shader = shader_manager.get("screen");

    // create vao
    glGenVertexArrays(1, &vao);
}

void PostProcessingStage::draw(GLFWwindow* window, const GLuint frame_texture) const {
    glUseProgram(screen_shader);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(1.f, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGlErrors();
    // Enabling alpha channel for textures
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // Draw the screen texture on the quad geometry
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    const GLuint screen_program = shader_manager.get("screen");

    // Set clock
    setUniformFloat(screen_program, "time", static_cast<float>(glfwGetTime() * 10.0f));

    // Set the vertex position and vertex texture coordinates (both stored in the same VBO)
    GLint position_location = glGetAttribLocation(screen_program, "in_position");
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)nullptr);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame_texture);

    // Draw
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();

    glBindVertexArray(0);
    glUseProgram(0);
}

PostProcessingStage::~PostProcessingStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    checkGlErrors();
}
