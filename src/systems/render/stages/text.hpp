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
    std::unordered_map<uint32_t, Character> characters;

    FT_Face face = nullptr;

    GLuint vao = 0;
    GLuint vbo = 0;

    ShaderHandle text_shader = 0;

    mat4 projection_matrix = {};

    GLFWwindow* window = nullptr;

    void initCharacters();

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