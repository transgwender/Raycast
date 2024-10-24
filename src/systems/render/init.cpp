#include "render.hpp"
#include <array>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"
#include "logging/log.hpp"

// This creates circular header inclusion, that is quite bad.
#include "../../ecs/registry.hpp"

#include <iostream>
#include <sstream>

// World initialization
bool RenderSystem::init(GLFWwindow* window_arg) {
    this->window = window_arg;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // Load OpenGL function pointers
    const int is_fine = gl3w_init();
    assert(is_fine == 0);

    // Create a frame buffer
    frame_buffer = 0;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    checkGlErrors();

    printf("OpenGL Frame Buffer Size = %d,%d\n", native_width, native_height);
    printf("Window Dimensions = %d,%d\n", window_width_px, window_height_px);

    // We are not really using VAOs but without at least one bound we will
    // crash in some systems.
    glGenVertexArrays(1, &base_vao);
    glBindVertexArray(base_vao);
    checkGlErrors();

    initScreenTexture();
    texture_manager.init();
    shader_manager.init();
    text.init();
    initializeGlGeometryBuffers();

    return true;
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOAndIBO(GEOMETRY_BUFFER gid, std::vector<T> vertices, std::vector<uint16_t> indices) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    checkGlErrors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
    checkGlErrors();
}

void RenderSystem::initializeGlGeometryBuffers() {
    // Vertex Buffer creation.
    glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
    // Index Buffer creation.
    glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

    //////////////////////////
    // Initialize sprite
    // The position corresponds to the center of the texture.
    std::vector<TexturedVertex> textured_vertices(4);
    textured_vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
    textured_vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
    textured_vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
    textured_vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
    textured_vertices[0].texcoord = {0.f, 1.f};
    textured_vertices[1].texcoord = {1.f, 1.f};
    textured_vertices[2].texcoord = {1.f, 0.f};
    textured_vertices[3].texcoord = {0.f, 0.f};

    // Counterclockwise as it's the default opengl front winding direction.
    const std::vector<uint16_t> textured_indices = {0, 3, 1, 1, 3, 2};
    bindVBOAndIBO(GEOMETRY_BUFFER::SPRITE, textured_vertices, textured_indices);

    // Initialize screen triangle (yes, triangle, not quad; its more efficient).
    std::vector<vec3> screen_vertices(3);
    screen_vertices[0] = {-1, -6, 0.f};
    screen_vertices[1] = {6, -1, 0.f};
    screen_vertices[2] = {-1, 6, 0.f};

    // Counterclockwise as it's the default opengl front winding direction.
    const std::vector<uint16_t> screen_indices = {0, 1, 2};
    bindVBOAndIBO(GEOMETRY_BUFFER::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem() {
    // Don't need to free gl resources since they last for as long as the
    // program, but it's polite to clean after yourself.
    glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
    glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
    glDeleteTextures(1, &off_screen_render_buffer_color);
    glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
    checkGlErrors();

    // delete allocated resources
    glDeleteFramebuffers(1, &frame_buffer);
    checkGlErrors();

    // remove all entities created by the render system
    while (!registry.renderables.entities.empty())
        registry.remove_all_components_of(registry.renderables.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture() {
    glGenTextures(1, &off_screen_render_buffer_color);
    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, native_width, native_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    checkGlErrors();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, off_screen_render_buffer_color, 0);

    glGenRenderbuffers(1, &off_screen_render_buffer_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, native_width, native_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
    checkGlErrors();

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
