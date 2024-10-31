// internal
#include "render.hpp"
#include "common.hpp"
#include <SDL.h>

#include "ecs/registry.hpp"

#include <iostream>

void RenderSystem::activeTexturedShader(const Entity entity, const std::string& texture,
                                        const GLuint program) const {
    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
    checkGlErrors();
    assert(in_texcoord_loc >= 0);

    if (registry.spriteSheets.has(entity)) {
        float posX = 0;
        float posY = 0;
        float spriteWidth = 32;
        float spriteHeight = 32;
        float texWidth = 384;
        float texHeight = 192;
        int frameIndex = 0;
        const float verts[] = {
            posX, posY,
            posX + spriteWidth, posY,
            posX + spriteWidth, posY + spriteHeight,
            posX, posY + spriteHeight
        };
        const float tw = float(spriteWidth) / texWidth;
        const float th = float(spriteHeight) / texHeight;
        const int numPerRow = texWidth / spriteWidth;
        const float tx = (frameIndex % numPerRow) * tw;
        const float ty = (frameIndex / numPerRow + 1) * th;
        const float texVerts[] = {
            tx, ty,
            tx + tw, ty,
            tx + tw, ty + th,
            tx, ty + th
        };

        // ... Bind the texture, enable the proper arrays
        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 2, GL_FLOAT, GL_FALSE, 0, verts);
        checkGlErrors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, texVerts);
        checkGlErrors();
    } else {
        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)nullptr);
        checkGlErrors();

        glEnableVertexAttribArray(in_texcoord_loc);
        glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));
        checkGlErrors();
    }

    // set uniforms
    auto albedoTexLocation = glGetUniformLocation(program, "albedo_tex");
    auto normalTexLocation = glGetUniformLocation(program, "normal_tex");
    glUniform1i(albedoTexLocation, 0);
    glUniform1i(normalTexLocation, 1);

    setUniformFloatVec3(program, "ambient_light", ambientLightColour / 255.0f);

    // uncomment this to have the first light ray follow your mouse
    // used for point light testing (will remove later)
    // if (!registry.lightRays.entities.empty()) {
    //     double xpos, ypos;
    //     glfwGetCursorPos(window, &xpos, &ypos);
    //     Motion& motion = registry.motions.get(registry.lightRays.entities[0]);
    //     motion.position = screenToWorld(vec2(xpos, ypos));
    // }

    int pointLightsCount = 0;
    for (const auto& point_light : registry.pointLights.entities) {
        PointLight& pl = registry.pointLights.get(point_light);
        Motion& motion = registry.motions.get(point_light);

        const int i = pointLightsCount++;
        const std::string prefix = std::string("point_lights[") +
                                   std::to_string(i) + std::string("].");

        setUniformFloatVec3(program, (prefix + std::string("position")).c_str(),
                            vec3(motion.position, 10.0f));
        setUniformFloatVec3(program, (prefix + std::string("diffuse")).c_str(),
                            pl.diffuse / 255.0f);
        setUniformFloat(program, (prefix + std::string("constant")).c_str(),
                        pl.constant);
        setUniformFloat(program, (prefix + std::string("linear")).c_str(),
                        pl.linear);
        setUniformFloat(program, (prefix + std::string("quadratic")).c_str(),
                        pl.quadratic);
    }

    auto pointLightsCountLocation =
        glGetUniformLocation(program, "point_lights_count");
    glUniform1i(pointLightsCountLocation, pointLightsCount);

    // Enabling and binding texture to slot 0 and 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_manager.get(texture));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_manager.getNormal(texture));
    checkGlErrors();

    bool isHighlighted = registry.highlightables.has(entity) && registry.highlightables.get(entity).isHighlighted;
    glUniform1i(glGetUniformLocation(program, "highlight"), isHighlighted ? 1 : 0);
}

void RenderSystem::drawTexturedMesh(const Entity entity) const {
    const auto& [position, angle, velocity, scale, collides] =
        registry.motions.get(entity);
    const auto& [texture, shader] = registry.materials.get(entity);

    Transform transform;
    transform.translate(position);
    transform.rotate(angle);
    transform.scale(scale);

    const auto program = shader_manager.get(shader);

    // Setting shaders
    glUseProgram(program);
    checkGlErrors();

    const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER::SPRITE];
    const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER::SPRITE];

    // Setting vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    checkGlErrors();

    if (shader == "textured") {
        activeTexturedShader(entity, texture, program);
    } else {
        assert(false && "Type of render request not supported");
    }

    // Getting uniform locations for glUniform* calls
    checkGlErrors();

    // Get number of indices from index buffer, which has elements uint16_t
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    checkGlErrors();

    GLsizei num_indices = size / sizeof(uint16_t);

    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    // Setting uniform values to the currently bound program
    GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
    checkGlErrors();
    // Drawing of num_indices/3 triangles specified in the index buffer
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    checkGlErrors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen() const {
    // Setting shaders
    // get the water texture, sprite mesh, and program
    glUseProgram(shader_manager.get("screen"));
    checkGlErrors();
    // Clearing back buffer
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
    glBindBuffer(GL_ARRAY_BUFFER,
                 vertex_buffers[(GLuint)GEOMETRY_BUFFER::SCREEN_TRIANGLE]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 index_buffers[(GLuint)GEOMETRY_BUFFER::SCREEN_TRIANGLE]);
    checkGlErrors();
    const GLuint screen_program = shader_manager.get("screen");
    // Set clock
    const GLuint time_uloc = glGetUniformLocation(screen_program, "time");
    glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
    checkGlErrors();
    // Set the vertex position and vertex texture coordinates (both stored in
    // the same VBO)
    GLint in_position_loc = glGetAttribLocation(screen_program, "in_position");
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3),
                          (void*)nullptr);
    checkGlErrors();

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
    checkGlErrors();
    // Draw
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    checkGlErrors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw() {
    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    checkGlErrors();
    // Clearing back buffer
    glViewport(0, 0, native_width, native_height);
    glDepthRange(0.00001, 10);
    glClearColor(GLfloat(0.0), GLfloat(0.0), GLfloat(0.0), 1.0);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
                              // and alpha blending, one would have to sort
                              // sprites back to front
    checkGlErrors();
    // Draw all textured meshes that have a position and size component
    for (const Entity entity : registry.materials.entities) {
        drawTexturedMesh(entity);
    }

    // Truly render to the screen
    drawToScreen();

    // flicker-free display with a double buffer
    glfwSwapBuffers(window);
    checkGlErrors();
}
