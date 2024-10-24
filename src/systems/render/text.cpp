#include "text.hpp"

#include "common.hpp"
#include "shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

// text rendering code adapted from https://learnopengl.com/In-Practice/Text-Rendering

bool TextSystem::init() {
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        LOG_ERROR("Failed to initialize FreeType");
        return false;
    }

    if (FT_New_Face(library, font_path("Silver.ttf").c_str(), 0, &face) != 0) {
        LOG_ERROR("Failed to load font");
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, 200);

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

void TextSystem::initCharacters() {
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
        Character character = {texture, ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<unsigned int>(face->glyph->advance.x)};
        characters.insert(std::pair<char, Character>(c, character));
    }
}

void TextSystem::renderText(ShaderHandle shader, const std::string& text, float x, float y, float scale, vec3 color) {
    glUseProgram(shader);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width_px, window_height_px);
    mat4 projection = ortho(0.0f, (float)window_width_px, 0.0f, (float)window_height_px);

    auto color_loc = glGetUniformLocation(shader, "textColor");
    glUniform3f(color_loc, color.x, color.y, color.z);
    GLuint projection_loc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (float*)&projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);
    checkGlErrors();

    // iterate through all characters
    for (char c : text) {
        Character ch = characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},
                                {xpos + w, ypos, 1.0f, 1.0f}, {xpos, ypos + h, 0.0f, 0.0f},
                                {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
