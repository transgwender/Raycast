#pragma once
#include "common.hpp"
#include "shader.hpp"

struct Character {
    unsigned int texture;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
};

class TextSystem {
    std::unordered_map<char, Character> characters;
    FT_Face face = nullptr;
    unsigned int vao;
    unsigned int vbo;

    void initCharacters();

  public:

    /**
     * Initialize the text rendering system.
     *
     * Returns true if everything was initialized properly, false otherwise.
     */
    bool init();

    void renderText(ShaderHandle shader, const std::string& text, float x, float y, float scale, vec3 color);
};