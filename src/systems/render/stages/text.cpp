#include "text.hpp"

#include "../shader.hpp"
#include "common.hpp"
#include "registry.hpp"
#include "render.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// text rendering code adapted from https://learnopengl.com/In-Practice/Text-Rendering

void TextStage::init() {
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        LOG_ERROR("Failed to initialize FreeType");
        return;
    }

    if (FT_New_Face(library, font_path("Silver.ttf").c_str(), 0, &face) != 0) {
        LOG_ERROR("Failed to load font");
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 200);

    text_shader = shader_manager.get("text");

    initCharacters();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextStage::initCharacters() {
    // disable byte alignment requirement
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (uint32_t c = 0; c < 128; c++) {
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
        Character character = {texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<unsigned int>(face->glyph->advance.x)};
        characters.insert(std::pair(c, character));
    }
}

void TextStage::renderText(const std::string& text, float x, float y, float scale, vec3 color) {
    glUseProgram(text_shader);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width_px, window_height_px);

    setUniformFloatVec3(text_shader, "textColor", color / 255.0f);
    setUniformFloatMat4(text_shader, "projection", projection_matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    checkGlErrors();

    // iterate through all characters
    for (const char c : text) {
        auto [texture, size, bearing, advance] = characters[c];

        float xpos = x + bearing.x * scale;
        float ypos = y - (size.y - bearing.y) * scale;

        float w = size.x * scale;
        float h = size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},
                                {xpos + w, ypos, 1.0f, 1.0f}, {xpos, ypos + h, 0.0f, 0.0f},
                                {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, texture);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    checkGlErrors();
}

void TextStage::draw() {
    projection_matrix = ortho(0.0f, (float)window_width_px, 0.0f, (float)window_height_px);
    auto world_width = static_cast<float>(native_width);
    auto world_height = static_cast<float>(native_height);

    for (const Text& text : registry.texts.components) {
        float x = (text.position.x / world_width) * window_width_px;
        float y = (text.position.y / world_height) * window_height_px;
        renderText(text.text, x, y, 1.0, text.color);
    }
}

TextStage::~TextStage() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
