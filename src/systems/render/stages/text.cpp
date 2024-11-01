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
    text_shader = shader_manager.get("text");

    initFont();
    initFrame();
    initCharacters();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // add this stage's frame texture to the texture manager
    texture_manager.add("$text_stage", frame_texture);
}

void TextStage::initFont() {
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        LOG_ERROR("Failed to initialize FreeType");
        return;
    }

    if (FT_New_Face(library, font_path("Silver.ttf").c_str(), 0, &face) != 0) {
        LOG_ERROR("Failed to load font");
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
}

void TextStage::initFrame() {
    // create a new framebuffer to render to
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // create a new screen texture and bind it to our new framebuffer
    glGenTextures(1, &frame_texture);
    glBindTexture(GL_TEXTURE_2D, frame_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);

    // go back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextStage::initCharacters() {
    // disable byte alignment requirement
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_ERROR("Failed to load glyph");
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
        characters[c] = character;
    }
}

void TextStage::renderText(const std::string& text, float x, float y, float scale, const vec3 color) {
    const float start_pos = x;

    glUseProgram(text_shader);
    glViewport(0, 0, frame_width, frame_height);

    setUniformFloatVec3(text_shader, "textColor", color / 255.0f);
    setUniformFloatMat4(text_shader, "projection", projection_matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    checkGlErrors();

    // iterate through all characters
    for (const unsigned char c : text) {
        auto [texture, size, bearing, advance] = characters[c];

        if (c == '\n') {
            y -= static_cast<float>(size.y) * scale;
            x = start_pos;
            continue;
        }
        if (c == ' ') {
            x += (advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
            continue;
        }

        float xpos = x + bearing.x * scale;
        float ypos = y - (size.y - bearing.y) * scale;

        float scale_x = size.x * scale;
        float scale_y = size.y * scale;

        auto transform = mat4(1.0f);
        transform *= translate(mat4(1.0f), vec3(xpos, ypos, 0.0f));
        transform *= glm::scale(mat4(1.0f), vec3(scale_x, scale_y, 0.0f));

        setUniformFloatMat4(text_shader, "transform", transform);

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, texture);

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // render quad
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
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
}

void TextStage::draw() {
    prepareDraw();

    const auto world_width = static_cast<float>(native_width);
    const auto world_height = static_cast<float>(native_height);

    for (const Text& text : registry.texts.components) {
        const float x = (text.position.x / world_width) * static_cast<float>(frame_width);
        const float y = (text.position.y / world_height) * static_cast<float>(frame_height);
        renderText(text.text, x, y, 1.0, text.color);
    }
}

TextStage::~TextStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
