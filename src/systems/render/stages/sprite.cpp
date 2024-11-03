#include "sprite.hpp"
#include "registry.hpp"
#include "render.hpp"

void SpriteStage::init() {
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

    shader = shader_manager.get("textured");

    createVertexAndIndexBuffers();

    // add this stage's frame texture to the texture manager
    texture_manager.add("$sprite_stage", frame_texture);
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
void SpriteStage::activateShader(const Entity& entity, const std::string& texture) const {
    // set uniforms
    setUniformInt(shader, "albedo_tex", 0);
    setUniformInt(shader, "normal_tex", 1);

    setUniformFloatVec3(shader, "ambient_light", ambient_light_colour / 255.0f);


    setUniformInt(shader, "skip_lighting", registry.menuItems.has(entity));

    int point_lights_count = 0;
    for (const auto& point_light : registry.pointLights.entities) {
        PointLight& pl = registry.pointLights.get(point_light);
        Motion& motion = registry.motions.get(point_light);

        const int i = point_lights_count++;
        const std::string prefix = std::string("point_lights[") + std::to_string(i) + std::string("].");

        setUniformFloatVec3(shader, (prefix + std::string("position")).c_str(), vec3(motion.position, 10.0f));
        setUniformFloatVec3(shader, (prefix + std::string("diffuse")).c_str(), pl.diffuse / 255.0f);
        setUniformFloat(shader, (prefix + std::string("constant")).c_str(), pl.constant);
        setUniformFloat(shader, (prefix + std::string("linear")).c_str(), pl.linear);
        setUniformFloat(shader, (prefix + std::string("quadratic")).c_str(), pl.quadratic);
    }

    setUniformInt(shader, "point_lights_count", point_lights_count);

    // Enabling and binding texture to slot 0 and 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_manager.get(texture));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_manager.getNormal(texture));
    checkGlErrors();

    const bool isHighlighted = registry.highlightables.has(entity) && registry.highlightables.get(entity).isHighlighted;
    setUniformInt(shader, "highlight", isHighlighted ? 1 : 0);
}

/**
 * Prepare for drawing by setting various OpenGL flags, setting and clearing the framebuffer,
 * and updating the viewport.
 */
void SpriteStage::prepareDraw() const {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glUseProgram(shader);
    setUniformFloatMat3(shader, "projection", projection_matrix);
    glViewport(0, 0, native_width, native_height);
    glDepthRange(0.00001, 10);
    glClearColor(static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), 1.0);
    glClearDepth(10.f);
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
    assert(texcoord_location >= 0);

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
void SpriteStage::drawSprite(const Entity entity, float elapsed_ms) {
    const auto& [position, angle, velocity, scale] = registry.motions.get(entity);
    const auto& texture = registry.materials.get(entity).texture;

    Transform transform;
    transform.translate(position);
    transform.rotate(angle);
    transform.scale(scale);

    activateShader(entity, texture);

    // Setting uniform values to the currently bound program
    setUniformFloatMat3(shader, "transform", transform.mat);
    setUniformFloatMat3(shader, "projection", projection_matrix);

    if (registry.spriteSheets.has(entity)) {
        SpriteSheet& ss = registry.spriteSheets.get(entity);
        ss.timeElapsed += elapsed_ms;


        if (registry.levers.has(entity)) { // IF animated sprite is a lever
            animateLever(entity, ss);

        } else { // If animated sprite is not a lever

            // Update animation frame
            if (ss.timeElapsed >= animation_speed) {
                ss.currFrame = (ss.currFrame + 1) % ss.animationFrames[ss.currState];
                ss.timeElapsed = 0.f;
            }
        }

         // Calculate UV coord offset
        float h_offset = ss.cellHeight * static_cast<float>(ss.currFrame) / ss.sheetWidth;
        float v_offset = ss.cellWidth * static_cast<float>(ss.currState) / ss.sheetHeight;
        vec2 cell_size = vec2(ss.cellWidth / ss.sheetWidth, ss.cellHeight / ss.sheetHeight);

        setUniformFloat(shader, "horizontal_offset", h_offset);
        setUniformFloat(shader, "vertical_offset", v_offset);
        setUniformFloatVec2(shader, "cell_size", cell_size); 

    } else {
        // Default texture coordinates
        setUniformFloatVec2(shader, "cell_size", vec2(1, 1));
        setUniformFloat(shader, "horizontal_offset", 0);
        setUniformFloat(shader, "vertical_offset", 0);
    }
    checkGlErrors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GLsizei num_indices = size / sizeof(uint16_t);

    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, std::size(textured_indices), GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();
}

void SpriteStage::animateLever(const Entity& entity, SpriteSheet& ss) {
    Lever& lever = registry.levers.get(entity);

    // Update animation frame
    if (ss.timeElapsed >= animation_speed) {
        if (lever.movementState == LEVER_MOVEMENT_STATES::PUSHED_RIGHT &&
            ss.currFrame < ss.animationFrames[ss.currState] - 1) {
            ss.currFrame = (ss.currFrame + 1) % ss.animationFrames[ss.currState];
            ss.timeElapsed = 0.f;
            lever.movementState = LEVER_MOVEMENT_STATES::STILL;
        }

        if (lever.movementState == LEVER_MOVEMENT_STATES::PUSHED_LEFT && ss.currFrame > 0) {
            ss.currFrame = (ss.currFrame - 1) % ss.animationFrames[ss.currState];
            ss.timeElapsed = 0.f;
            lever.movementState = LEVER_MOVEMENT_STATES::STILL;
        }

        lever.state = (LEVER_STATES) ss.currFrame;
    }
}

void SpriteStage::draw(float elapsed_ms) {
    prepareDraw();

    // Draw all textured meshes that have a material and motion component
    for (const Entity& entity : registry.materials.entities) {
        drawSprite(entity, elapsed_ms);
    }
}

SpriteStage::~SpriteStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    glDeleteTextures(1, &frame_texture);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &frame_buffer);

    checkGlErrors();
}
