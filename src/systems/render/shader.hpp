#pragma once
#include "common.hpp"
#include <string>
#include <unordered_map>

typedef GLuint ShaderHandle;

class ShaderManager {
    bool initialized = false;
    std::unordered_map<std::string, ShaderHandle> shaders;

    ShaderHandle add(const std::string& name);

  public:
    /**
     * Create a new shader manager instance. This loads all the vertex-fragment shader pairs in the
     * shader folder and compiles them into a program, which can then be referenced at any time.
     *
     * Using this class without calling `init` first will lead to undefined behaviour.
     */
    void init();

    [[nodiscard]] ShaderHandle get(const std::string& name) const;

    ~ShaderManager();
};

void setUniformFloat(GLuint program, const char* name, float value);

void setUniformInt(GLuint program, const char* name, int value);

void setUniformFloatVec2(GLuint program, const char* name, vec2 value);

void setUniformFloatVec3(GLuint program, const char* name, vec3 value);

void setUniformFloatVec4(GLuint program, const char* name, vec4 value);

void setUniformFloatMat3(GLuint program, const char* name, mat3 value);

void setUniformFloatMat4(GLuint program, const char* name, mat4 value);
