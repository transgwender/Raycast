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
    const std::string font_name = "Silver.ttf";

    /** Maps from a font size to a character set. */
    std::unordered_map<unsigned int, std::vector<Character>> character_sets;

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

    GLfloat quad_vertices[4][4] = {
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 1.0f}
    };

    void initFont();

    /**
     * Initialize the frame buffer and associated texture that this stage
     * will render to.
     */
    void initFrame();

    void createCharacterSet(unsigned int size);

    std::vector<Character>& getCharacterSet(unsigned int size);

    void prepareDraw();

    void renderText(const std::string& text, float x, float y, unsigned int size, vec3 color, bool centered);

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