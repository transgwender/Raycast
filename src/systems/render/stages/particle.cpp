#include "particle.hpp"

#include "registry.hpp"
#include "render.hpp"

void ParticleStage::createVertexAndIndexBuffers() {
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textured_vertices), textured_vertices, GL_STATIC_DRAW);

    checkGlErrors();
}

void ParticleStage::init() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create a new vao
    glGenVertexArrays(1, &vao);

    frame_texture = texture_manager.get("$world_texture");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    checkGlErrors();

    createVertexAndIndexBuffers();

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    updateShaders();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    checkGlErrors();
}

/**
 * Prepare for drawing by setting various OpenGL flags, setting and clearing the framebuffer,
 * and updating the viewport.
 */
void ParticleStage::prepareDraw() const {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glUseProgram(shader);
    setUniformFloatMat3(shader, "projection", projection_matrix);
    glViewport(0, 0, native_width, native_height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    checkGlErrors();
}

/**
 * Update uniform variables for the shader program based on the given entity and texture.
 */
void ParticleStage::activateShader(const Particle& particle) const {
    // Enabling and binding texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particle.texture);

    setUniformFloatVec4(shader, "texture_color", particle.color / 255.0f);
}

void ParticleStage::draw() const {
    prepareDraw();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (const Particle& particle : registry.particles.components) {
        Transform transform;
        transform.translate(particle.position);
        transform.rotate(particle.angle);
        transform.scale(particle.scale);

        activateShader(particle);

        setUniformFloatMat3(shader, "transform", transform.mat);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void ParticleStage::updateShaders() {
    shader = shader_manager.get("particle");
}

ParticleStage::~ParticleStage() {
    glDeleteBuffers(1, &vbo);

    glDeleteTextures(1, &frame_texture);
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &vao);

    checkGlErrors();
}
