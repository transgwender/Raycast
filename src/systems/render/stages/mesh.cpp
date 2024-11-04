#include "mesh.hpp"
#include "registry.hpp"
#include "render.hpp"

void MeshStage::init() {
    createFrame();
    shader = shader_manager.get("mesh");
}

void MeshStage::createFrame() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create a new screen texture and bind it to our new framebuffer
    glGenTextures(1, &frame_texture);
    glBindTexture(GL_TEXTURE_2D, frame_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, native_width, native_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // add this stage's frame texture to the texture manager
    texture_manager.add("$mesh_stage", frame_texture);

    checkGlErrors();
}

void MeshStage::addMesh(const Entity& entity) {
    const Mesh& mesh = registry.meshes.get(entity);

    GLuint vbo, ibo, vao;

    // create a new vao
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mesh.vertices[0]) * mesh.vertices.size(), mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh.vertex_indices[0]) * mesh.vertex_indices.size(),
                 mesh.vertex_indices.data(), GL_STATIC_DRAW);

    checkGlErrors();

    vaos[entity] = vao;
    vbos[entity] = vbo;
    ibos[entity] = ibo;
}

/**
 * Update uniform variables for the shader program based on the given entity and texture.
 */
void MeshStage::activateShader(const Entity& entity) {
    glUseProgram(shader);
    setUniformFloatVec3(shader, "fcolor", vec3(1.0f, 1.0f, 1.0f));
    setUniformFloatMat3(shader, "projection", projection_matrix);
}

/**
 * Prepare for drawing by setting various OpenGL flags, setting and clearing the framebuffer,
 * and updating the viewport.
 */
void MeshStage::prepareDraw() {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    glViewport(0, 0, native_width, native_height);
    glDepthRange(0.00001, 10);
    glClearColor(static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), static_cast<GLfloat>(0.0), 0.0);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}

void MeshStage::activateMesh(const Entity& mesh_entity) {
    if (vaos.find(mesh_entity) == vaos.end()) {
        addMesh(mesh_entity);
    }

    // Setting active vertex and index buffers
    GLuint vao = vaos.at(mesh_entity);
    GLuint vbo = vbos.at(mesh_entity);
    GLuint ibo = ibos.at(mesh_entity);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    GLint position_location = glGetAttribLocation(shader, "in_position");
    GLint color_location = glGetAttribLocation(shader, "in_color");
    assert(color_location >= 0);

    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)nullptr);
    glEnableVertexAttribArray(color_location);
    glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)sizeof(vec3));

    checkGlErrors();
}

/**
 * Draw a given Entity which has a `Motion` and `Material`.
 * @param entity The sprite to render
 */
void MeshStage::drawMesh(const Entity& entity) {
    const auto& [position, angle, velocity, scale] = registry.motions.get(entity);

    Transform transform;
    transform.translate(position);
    transform.rotate(angle);
    transform.scale(scale);

    // Setting uniform values to the currently bound program
    setUniformFloatMat3(shader, "transform", transform.mat);

    // Drawing of num_indices/3 triangles specified in the index buffer
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GLsizei num_indices = size / sizeof(uint16_t);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);

    checkGlErrors();
}

void MeshStage::draw() {
    prepareDraw();

    for (const Entity& mesh_entity : registry.meshes.entities) {
        activateShader(mesh_entity);
        activateMesh(mesh_entity);
        drawMesh(mesh_entity);
    }
}

MeshStage::~MeshStage() {
    glDeleteTextures(1, &frame_texture);
    glDeleteFramebuffers(1, &frame_buffer);

    checkGlErrors();
}
