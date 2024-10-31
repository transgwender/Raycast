#include "shader.hpp"
#include "common.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

bool compileShader(const GLuint shader) {
    glCompileShader(shader);
    checkGlErrors();
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(log_len);
        glGetShaderInfoLog(shader, log_len, &log_len, log.data());
        glDeleteShader(shader);

        checkGlErrors();

        fprintf(stderr, "GLSL: %s", log.data());
        return false;
    }

    return true;
}

bool loadShader(const std::string& vs_path, const std::string& fs_path, GLuint& out_program) {
    // Opening files
    std::ifstream vs_is(vs_path);
    std::ifstream fs_is(fs_path);
    if (!vs_is.good() || !fs_is.good()) {
        fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
        return false;
    }

    // Reading sources
    std::stringstream vs_ss, fs_ss;
    vs_ss << vs_is.rdbuf();
    fs_ss << fs_is.rdbuf();
    std::string vs_str = vs_ss.str();
    std::string fs_str = fs_ss.str();
    const char* vs_src = vs_str.c_str();
    const char* fs_src = fs_str.c_str();
    auto vs_len = (GLsizei)vs_str.size();
    auto fs_len = (GLsizei)fs_str.size();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs_src, &vs_len);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs_src, &fs_len);
    checkGlErrors();

    // Compiling
    if (!compileShader(vertex)) {
        fprintf(stderr, "Vertex compilation failed");
        return false;
    }
    if (!compileShader(fragment)) {
        fprintf(stderr, "Vertex compilation failed");
        return false;
    }

    // Linking
    out_program = glCreateProgram();
    glAttachShader(out_program, vertex);
    glAttachShader(out_program, fragment);
    glLinkProgram(out_program);
    checkGlErrors();

    {
        GLint is_linked = GL_FALSE;
        glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
        if (is_linked == GL_FALSE) {
            GLint log_len;
            glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
            std::vector<char> log(log_len);
            glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
            checkGlErrors();

            fprintf(stderr, "Link error: %s", log.data());
            return false;
        }
    }

    // No need to carry this around. Keeping these objects is only useful if we
    // recycle the same shaders over and over, which we don't, so no need and
    // this is simpler.
    glDetachShader(out_program, vertex);
    glDetachShader(out_program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    checkGlErrors();

    return true;
}

ShaderHandle ShaderManager::add(const std::string& name) {
    const std::string vs_path = shader_path(name + ".vs.glsl");
    const std::string fs_path = shader_path(name + ".fs.glsl");
    GLuint program;
    if (!loadShader(vs_path, fs_path, program)) {
        std::cout << "Failed to load shader at paths " << vs_path << " and " << fs_path << std::endl;
        assert(false);
    }
    shaders[name] = program;
    return program;
}

ShaderHandle ShaderManager::get(const std::string& name) const {
    if (shaders.find(name) == shaders.end()) {
        std::cout << "Shader not found: " << name
                  << ". Double check your spelling and make sure you added the "
                     "shader first."
                  << std::endl;
    }
    return shaders.at(name);
}

void ShaderManager::init() {
    if (initialized)
        return;
    for (const auto& entry : std::filesystem::directory_iterator(shader_path(""))) {
        const auto& path = entry.path();
        const std::string path_string = path.stem().string();
        const std::string shader_name = path_string.substr(0, path_string.find_first_of('.'));
        if (shaders.find(shader_name) == shaders.end()) {
            add(shader_name);
        }
    }
    initialized = true;
}

ShaderManager::~ShaderManager() {
    for (const auto& [_, program] : shaders) {
        glDeleteProgram(program);
    }
}

void setUniformFloat(const GLuint program, const char* name, float value) {
    const GLint location = glGetUniformLocation(program, name);
    glUniform1f(location, value);
}

void setUniformFloatVec3(const GLuint program, const char* name, const vec3 value) {
    const GLint location = glGetUniformLocation(program, name);
    glUniform3fv(location, 1, (float*)&value);
}

void setUniformFloatMat3(const GLuint program, const char* name, const mat3 value) {
    const GLint location = glGetUniformLocation(program, name);
    glUniformMatrix3fv(location, 1, GL_FALSE, (float*)&value);
}
