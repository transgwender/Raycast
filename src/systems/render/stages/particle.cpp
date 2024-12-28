#include "particle.hpp"

#include "registry.hpp"
#include "render.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <optional>

void ParticleStage::createBuffers() {
    LOG_INFO("b1")
    glGenBuffers(1, &quad_vbo);
    LOG_INFO("b2")
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    LOG_INFO("b3")
    glBufferData(GL_ARRAY_BUFFER, sizeof(textured_vertices), textured_vertices, GL_STATIC_DRAW);
    LOG_INFO("b4")

    glGenBuffers(1, &instance_vbo);
    LOG_INFO("b5")
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    LOG_INFO("b6")
    allocateInstanceBuffer();
    LOG_INFO("b7")

    checkGlErrors();
}

void ParticleStage::allocateInstanceBuffer() const {
    const GLsizeiptr instance_buffer_size_bytes = sizeof(ParticleGPUData) * instance_buffer_size;
    glBufferData(GL_ARRAY_BUFFER, instance_buffer_size_bytes, nullptr, GL_STREAM_DRAW);
}

void ParticleStage::initVAO() {
    glBindVertexArray(vao);
    LOG_INFO("a1")

    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    LOG_INFO("a2")

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    LOG_INFO("a3")
    glEnableVertexAttribArray(0);
    LOG_INFO("a4")

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    LOG_INFO("a5")
    glEnableVertexAttribArray(1);
    LOG_INFO("a6")

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    LOG_INFO("a7")

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleGPUData), (void*)0);
    LOG_INFO("a8")
    glEnableVertexAttribArray(2);
    LOG_INFO("{}", (void*)glEnableVertexAttribArray);
    LOG_INFO("a9")
    checkGlErrors();
    LOG_INFO("{}", (void*)glVertexAttribDivisor);
    glVertexAttribDivisor(2, 1);
    checkGlErrors();
    LOG_INFO("a10")

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleGPUData), (void*)offsetof(ParticleGPUData, scale));
    LOG_INFO("a11")
    glEnableVertexAttribArray(3);
    LOG_INFO("a12")
    glVertexAttribDivisor(3, 1);
    LOG_INFO("a13")

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleGPUData), (void*)offsetof(ParticleGPUData, color));
    LOG_INFO("a14")
    glEnableVertexAttribArray(4);
    LOG_INFO("a15")
    glVertexAttribDivisor(4, 1);
    LOG_INFO("a16")

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleGPUData), (void*)offsetof(ParticleGPUData, angle));
    LOG_INFO("a17")
    glEnableVertexAttribArray(5);
    LOG_INFO("a18")
    glVertexAttribDivisor(5, 1);
    LOG_INFO("a19")
}

void ParticleStage::init() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    LOG_INFO("Initialized particle framebuffers")

    frame_texture = texture_manager.get("$world_texture");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);
    LOG_INFO("Initialized particle frame texture")

    glGenVertexArrays(1, &vao);
    LOG_INFO("Generated VAO")
    createBuffers();
    LOG_INFO("Created Buffers")
    initVAO();
    LOG_INFO("Created VAO")

    updateShaders();
    LOG_INFO("Initialized particle shaders")

    // go back to defaults
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    LOG_INFO("Finished cleaning up")

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

void ParticleStage::draw() {
    prepareDraw();

    glBindVertexArray(vao);

    std::vector<std::optional<std::vector<ParticleGPUData>>> particle_groups;

    const auto width = static_cast<float>(native_width);
    const auto height = static_cast<float>(native_height);
    for (const Particle& particle : registry.particles.components) {
        const auto pos = particle.position;
        const auto scale = particle.scale;
        // bounds check
        if (pos.x - scale.x > width || pos.x + scale.x < 0.0 || pos.y - scale.y > height || pos.y + scale.y < 0.0)
            continue;

        ParticleGPUData p = {pos, scale, particle.color, particle.angle};

        if (particle.texture >= particle_groups.size()) {
            particle_groups.resize(particle.texture + 1);
        }

        if (particle_groups[particle.texture].has_value()) {
            particle_groups[particle.texture].value().emplace_back(p);
        } else {
            auto new_vec = std::vector<ParticleGPUData>();
            new_vec.emplace_back(p);
            particle_groups[particle.texture] = new_vec;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

    for (VirtualTextureHandle i = 0; i < particle_groups.size(); i++) {
        const auto& opt = particle_groups[i];
        if (!opt.has_value()) continue;
        const auto& group = opt.value();

        if (group.size() > instance_buffer_size) {
            instance_buffer_size = group.size() * 1.5;
            allocateInstanceBuffer();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_manager.get(i));
        glBufferSubData(GL_ARRAY_BUFFER, 0, group.size() * sizeof(ParticleGPUData), group.data());
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, group.size());
    }
}

void ParticleStage::updateShaders() { shader = shader_manager.get("particle"); }

ParticleStage::~ParticleStage() {
    glDeleteBuffers(1, &quad_vbo);

    glDeleteTextures(1, &frame_texture);
    glDeleteFramebuffers(1, &frame_buffer);
    glDeleteVertexArrays(1, &vao);

    checkGlErrors();
}
