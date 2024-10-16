#pragma once
#include <string>
#include <unordered_map>
#include "common.hpp"

typedef GLuint ShaderHandle;

class ShaderManager {
    std::unordered_map<std::string, ShaderHandle> shaders;

    ShaderHandle add(const std::string& name);

public:
    void init();

    [[nodiscard]] ShaderHandle get(const std::string& name) const;

    ~ShaderManager();
};

void setUniformFloat(GLuint program, const char* name, float value);

void setUniformFloatVec3(GLuint program, const char* name, vec3 value);
