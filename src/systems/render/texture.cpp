#include "texture.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void TextureManager::addFromPath(const std::filesystem::path& path) {
    const std::string texture_name = path.stem().string();
    if (texture_name.rfind('$', 0) == 0) {
        LOG_ERROR("Texture with name {} in the textures folder starts with reserved prefix character '$'. Please "
                  "rename the texture to something else.",
                  texture_name);
        assert(false);
    }

    ivec2 dimensions;
    stbi_uc* data = stbi_load(path.string().c_str(), &dimensions.x, &dimensions.y, nullptr, 4);
    if (data == nullptr) {
        LOG_WARN("Failed to read file {}", path.string());
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkGlErrors();
    stbi_image_free(data);
    textures[texture_name] = texture;
}

void TextureManager::init() {
    if (initialized)
        return;

    for (const auto& entry : std::filesystem::directory_iterator(textures_path("/albedo"))) {
        addFromPath(entry.path());
    }

    for (const auto& entry : std::filesystem::directory_iterator(textures_path("/normal"))) {
        addFromPath(entry.path());
    }

    initialized = true;
}

TextureHandle TextureManager::get(const std::string& name) const {
    if (textures.find(name) == textures.end()) {
        LOG_ERROR("The texture '{}' doesn't exist. Please check the spelling and make sure to leave out the file "
                  "extension.",
                  name)
        assert(false);
    }
    return textures.at(name);
}

TextureHandle TextureManager::getNormal(const std::string& name) const {
    const std::string normal_name = name + "_n";
    if (textures.find(normal_name) == textures.end()) {
        return textures.at("default_n");
    }
    return textures.at(normal_name);
}

void TextureManager::add(const std::string& name, const TextureHandle& texture) {
    if (textures.find(name) != textures.end()) {
        LOG_WARN("Texture with name '{}' already exists. Adding it again will overwrite the texture ID.", name);
    }
    textures[name] = texture;
}
