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

    // create a new screen texture and bind it to our new framebuffer
    glGenTextures(1, &frame_texture);
    glBindTexture(GL_TEXTURE_2D, frame_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, native_width, native_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    checkGlErrors();

    shader = shader_manager.get("particle");

    createVertexAndIndexBuffers();

    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    checkGlErrors();

    // add this stage's frame texture to the texture manager
    texture_manager.add("$particle_stage", frame_texture);
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
    glDepthRange(0.00001, 10);
    glClearColor(static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), 0.0);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    checkGlErrors();
}

/**
 * Update uniform variables for the shader program based on the given entity and texture.
 */
void ParticleStage::activateShader(const TextureHandle& texture) const {
    // Enabling and binding texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void ParticleStage::draw() {
    prepareDraw();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int last_texture = -1;

    for (const Particle& particle : registry.particles.components) {
        Transform transform;
        transform.translate(particle.position);
        transform.rotate(particle.angle);
        transform.scale(particle.scale);

        if (last_texture != particle.texture) {
            activateShader(particle.texture);
            last_texture = particle.texture;
        }

        setUniformFloatMat3(shader, "transform", transform.mat);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

ParticleStage::~ParticleStage() {
    glDeleteBuffers(1, &vbo);

    glDeleteTextures(1, &frame_texture);
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &vao);

    checkGlErrors();
}
