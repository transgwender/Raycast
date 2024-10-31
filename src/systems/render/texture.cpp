#include "texture.hpp"

#include <filesystem>
#include <iostream>
#include <stb_image.h>
#include <string>

void TextureManager::addFromPath(const std::filesystem::path& path) {
    ivec2 dimensions;
    stbi_uc* data = stbi_load(path.string().c_str(), &dimensions.x, &dimensions.y, nullptr, 4);
    if (data == nullptr) {
        LOG_ERROR("Failed to read file {}", path.string());
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
    textures[path.stem().string()] = texture;
}

void TextureManager::init() {
    for (const auto& entry : std::filesystem::directory_iterator(textures_path("/albedo"))) {
        addFromPath(entry.path());
    }

    for (const auto& entry : std::filesystem::directory_iterator(textures_path("/normal"))) {
        addFromPath(entry.path());
    }
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
