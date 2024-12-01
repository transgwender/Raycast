#include "text.hpp"

#include "../shader.hpp"
#include "common.hpp"
#include "registry.hpp"
#include "render.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
 * Text rendering code adapted from https://learnopengl.com/In-Practice/Text-Rendering
 * Additional features and optimizations adapted from John from Whatever's Right Studios
 * https://www.youtube.com/watch?v=S0PyZKX4lyI (skipping spaces, adding newlines, uploading vertex buffer once)
 */

void TextStage::init() {
    initFont();
    initFrame();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    updateShaders();

    checkGlErrors();
}

void TextStage::initFont() {
    if (FT_Init_FreeType(&library) != 0) {
        LOG_ERROR("Failed to initialize FreeType");
        return;
    }

    if (FT_New_Face(library, font_path(font_name).c_str(), 0, &face) != 0) {
        LOG_ERROR("Failed to load font");
    }
}

void TextStage::initFrame() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create a new screen texture and bind it to our new framebuffer
    glGenTextures(1, &world_text_texture);
    glBindTexture(GL_TEXTURE_2D, world_text_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_text_texture, 0);

    glGenTextures(1, &ui_text_texture);
    glBindTexture(GL_TEXTURE_2D, ui_text_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ui_text_texture, 0);

    const GLenum draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, draw_buffers);

    // add this stage's frame textures to the texture manager
    texture_manager.add("$world_text", world_text_texture);
    texture_manager.add("$ui_text", ui_text_texture);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGlErrors();
}

void TextStage::createCharacterSet(const unsigned int size) {
    // disable byte alignment requirement
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Set_Pixel_Sizes(face, 0, size);

    std::vector<Character> characters;

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_ERROR("Failed to load glyph for character {}", c);
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        const Character character = {texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                     ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                     static_cast<unsigned int>(face->glyph->advance.x)};
        characters.push_back(character);
    }
    character_sets[size] = characters;
}

std::vector<Character>& TextStage::getCharacterSet(const unsigned int size) {
    if (character_sets.find(size) == character_sets.end()) {
        LOG_INFO("Creating font map of size {}", size);
        createCharacterSet(size);
    }
    return character_sets[size];
}

void TextStage::renderText(const Text& text, float x, float y) {
    glUseProgram(shader);
    glViewport(0, 0, frame_width, frame_height);

    setUniformFloatVec3(shader, "textColor", text.color / 255.0f);
    setUniformFloatMat4(shader, "projection", projection_matrix);
    setUniformInt(shader, "layer", text.layer);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    checkGlErrors();

    auto characters = getCharacterSet(text.size);

    float start_pos_x = x;
    float start_pos_y = y;

    if (text.centered) {
        float max_x = 0;
        float max_y = 0;

        // get width info
        for (const unsigned char c : text.text) {
            const float scale = 1.0;
            auto [texture, size, bearing, advance] = characters[c];
            if (c == '\n') {
                y += static_cast<float>(size.y) * scale;
                x = start_pos_x;
                continue;
            }
            if (c == ' ') {
                x += static_cast<float>(advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
                continue;
            }
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += static_cast<float>(advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

            max_x = max(max_x, x);
            max_y = max(max_y, y - static_cast<float>(size.y) * scale);
        }

        start_pos_x = start_pos_x - ((max_x - start_pos_x)/2.f);
        x = start_pos_x;
        start_pos_y = start_pos_y - ((max_y - start_pos_y)/2.f);
        y = start_pos_y;
    }

    // iterate through all characters
    for (const unsigned char c : text.text) {
        const float scale = 1.0;
        auto [texture, size, bearing, advance] = characters[c];

        if (c == '\n') {
            y += static_cast<float>(size.y) * scale;
            x = start_pos_x;
            continue;
        }
        if (c == ' ') {
            x += static_cast<float>(advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
            continue;
        }

        const float pos_x = x + static_cast<float>(bearing.x) * scale;
        const float pos_y = static_cast<float>(frame_height) - y - static_cast<float>(size.y - bearing.y) * scale;

        const float scale_x = static_cast<float>(size.x) * scale;
        const float scale_y = static_cast<float>(size.y) * scale;

        auto transform = mat4(1.0f);
        transform *= translate(mat4(1.0f), vec3(pos_x, pos_y, 0.0f));
        transform *= glm::scale(mat4(1.0f), vec3(scale_x, scale_y, 0.0f));

        setUniformFloatMat4(shader, "transform", transform);

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, texture);

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // render quad
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += static_cast<float>(advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    checkGlErrors();
}

void TextStage::prepareDraw() {
    projection_matrix = ortho(0.0f, static_cast<float>(frame_width), 0.0f, static_cast<float>(frame_height));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glViewport(0, 0, frame_width, frame_height);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void TextStage::draw() {
    prepareDraw();

    const auto world_width = static_cast<float>(native_width);
    const auto world_height = static_cast<float>(native_height);

    for (const Text& text : registry.texts.components) {
        if (text.layer == UI_TEXT) continue;
        const float x = (text.position.x / world_width) * static_cast<float>(frame_width);
        const float y = (text.position.y / world_height) * static_cast<float>(frame_height);
        renderText(text, x, y);
    }

    for (const Text& text : registry.texts.components) {
        if (text.layer == WORLD_TEXT) continue;
        const float x = (text.position.x / world_width) * static_cast<float>(frame_width);
        const float y = (text.position.y / world_height) * static_cast<float>(frame_height);
        renderText(text, x, y);
    }
}

void TextStage::updateShaders() {
    shader = shader_manager.get("text");
}


TextStage::~TextStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // Destroy &Face FIRST and then &FreeType because face is a child reference of library.
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}
