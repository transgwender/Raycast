#include "registry.hpp"
#include "render.hpp"
#include "sprite.hpp"

void SpriteStage::init() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create a new vao
    glGenVertexArrays(1, &vao);

    // create new render textures and bind it to our new framebuffer
    glGenTextures(1, &world_texture);
    glBindTexture(GL_TEXTURE_2D, world_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, native_width, native_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_texture, 0);

    glGenTextures(1, &ui_texture);
    glBindTexture(GL_TEXTURE_2D, ui_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, native_width, native_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ui_texture, 0);

    const GLenum draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    checkGlErrors();

    createVertexAndIndexBuffers();

    updateShaders();

    // add this stage's frame texture to the texture manager
    texture_manager.add("$world_texture", world_texture);
    texture_manager.add("$ui_texture", ui_texture);
}

void SpriteStage::createVertexAndIndexBuffers() {
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textured_vertices[0]) * std::size(textured_vertices), textured_vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(textured_indices[0]) * std::size(textured_indices), textured_indices,
                 GL_STATIC_DRAW);

    checkGlErrors();
}

/**
 * Update uniform variables for the shader program based on the given entity and texture.
 */
void SpriteStage::activateShader(const Entity& entity, const Motion& motion, const Material& material) const {
    Transform transform;
    transform.translate(motion.position);
    transform.rotate(motion.angle);
    transform.scale(motion.scale);

    // Setting uniform values to the currently bound program
    setUniformFloatMat3(shader, "transform", transform.mat);

    setUniformInt(shader, "layer", material.layer);

    setUniformFloatVec4(shader, "fcolor", material.color / 255.0f);
    setUniformInt(shader, "blend_mode", material.blend_mode);

    setUniformFloat(shader, "horizontal_offset", material.texture.h_offset);
    setUniformFloat(shader, "vertical_offset", material.texture.v_offset);
    setUniformFloatVec2(shader, "cell_size", material.texture.cell_size);

    // Enabling and binding texture to slot 0 and 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.texture.albedo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.texture.normal);

    const bool isHighlighted = registry.highlightables.has(entity) && registry.highlightables.get(entity).isHighlighted;
    setUniformInt(shader, "highlight", isHighlighted ? 1 : 0);

    checkGlErrors();
}

/**
 * Prepare for drawing by setting various OpenGL flags, setting and clearing the framebuffer,
 * and updating the viewport.
 */
void SpriteStage::prepareDraw() const {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glUseProgram(shader);

    setUniformInt(shader, "albedo_tex", 0);
    setUniformInt(shader, "normal_tex", 1);

    if (registry.ambientLights.size() == 0) {
        LOG_WARN("No ambient light found, using default ambient light.");
        registry.ambientLights.insert(Entity(), {default_ambient_light_colour});
        setUniformFloatVec3(shader, "ambient_light", default_ambient_light_colour / 255.0f);
    } else {
        setUniformFloatVec3(shader, "ambient_light", registry.ambientLights.components[0].color / 255.0f);
    }
    setUniformFloatMat3(shader, "projection", projection_matrix);

    // set point lights
    int point_lights_count = 0;
    for (const auto& point_light : registry.pointLights.entities) {
        PointLight& pl = registry.pointLights.get(point_light);
        Motion& motion = registry.motions.get(point_light);

        const int i = point_lights_count++;
        const std::string prefix = std::string("point_lights[") + std::to_string(i) + std::string("].");

        // point lights have hard coded z-value of 10
        setUniformFloatVec3(shader, (prefix + std::string("position")).c_str(), vec3(motion.position, 10.0f));
        setUniformFloatVec3(shader, (prefix + std::string("diffuse")).c_str(), pl.diffuse / 255.0f);
        setUniformFloat(shader, (prefix + std::string("constant")).c_str(), pl.constant);
        setUniformFloat(shader, (prefix + std::string("linear")).c_str(), pl.linear);
        setUniformFloat(shader, (prefix + std::string("quadratic")).c_str(), pl.quadratic);
    }

    setUniformInt(shader, "point_lights_count", point_lights_count);

    glViewport(0, 0, native_width, native_height);
    glDepthRange(0.0, 1.0);
    glClearColor(static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), 0.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Setting active vertex and index buffers
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    GLint position_location = glGetAttribLocation(shader, "in_position");
    GLint texcoord_location = glGetAttribLocation(shader, "in_texcoord");

    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)nullptr);
    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

    checkGlErrors();
}

/**
 * Draw a given Entity which has a `Motion` and `Material`.
 * @param entity The sprite to render
 */
void SpriteStage::drawSprite(const Entity& entity, const Material& material) const {
    const auto& motion = registry.motions.get(entity);

    activateShader(entity, motion, material);

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(std::size(textured_indices)), GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();
}

void SpriteStage::draw() const {
    prepareDraw();

    registry.materials.sort([](const Entity& e1, const Entity& e2) {
        auto& m1 = registry.materials.get(e1);
        auto& m2 = registry.materials.get(e2);
        return m1.layer < m2.layer;
    });

    for (int i = 0; i < registry.materials.size(); i++) {
        const Entity& entity = registry.materials.entities[i];
        const auto& material = registry.materials.components[i];
        if (!registry.invisibles.has(entity)) {
            drawSprite(entity, material);
        }
    }
}

void SpriteStage::updateShaders() {
    shader = shader_manager.get("textured");
}

SpriteStage::~SpriteStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    glDeleteTextures(1, &world_texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &frame_buffer);

    checkGlErrors();
}
