#pragma once
#include "../shader.hpp"
#include "common.hpp"

struct Character {
    unsigned int texture;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
};

class TextStage {
    /** Maps from a font size to a character set. */
    std::unordered_map<unsigned int, Character[128]> character_sets;
    Character characters[128] = {};

    FT_Face face = nullptr;

    GLuint vao = 0;
    GLuint vbo = 0;

    GLuint frame_buffer = 0;
    GLuint frame_texture = 0;

    // render text at this specific resolution.
    // this is much higher than 320x180, so the text will look sharper
    const int frame_width = 1280;
    const int frame_height = 720;

    ShaderHandle text_shader = 0;

    mat4 projection_matrix = {};

    void initFont();

    /**
     * Initialize the frame buffer and associated texture that this stage
     * will render to.
     */
    void initFrame();

    void initCharacters();

    void prepareDraw();

    void renderText(const std::string& text, float x, float y, float scale, vec3 color);

  public:
    /**
     * Initialize the text rendering system.
     *
     * Returns true if everything was initialized properly, false otherwise.
     */
    void init();

    void draw();

    ~TextStage();
};