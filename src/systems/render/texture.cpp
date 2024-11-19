#include "texture.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void TextureManager::addFromPath(const std::filesystem::directory_entry& entry) {
    const auto& path = entry.path();
    const std::string texture_name = path.stem().string();

    if (!path.has_extension())
        return;
    const std::string extension = path.extension().string();
    if (extension[extension.size() - 1] == '~')
        return;

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

    if (textures.find(texture_name) != textures.end()) {
        LOG_INFO("Updating texture '{}'", texture_name);
        glDeleteTextures(1, &textures[texture_name]);
    }

    texture_count++;
    write_times[texture_name] = entry.last_write_time();
    textures[texture_name] = texture;
    name_to_virtual[texture_name] = texture_count;
    virtual_to_actual[texture_count] = texture;
}

void TextureManager::init() {
    if (initialized)
        return;

    const auto albedo_folder = std::filesystem::directory_entry(textures_path("albedo"));
    last_albedo_write_time = albedo_folder.last_write_time();
    const auto normal_folder = std::filesystem::directory_entry(textures_path("normal"));
    last_normal_write_time = normal_folder.last_write_time();

    for (const auto& entry : std::filesystem::directory_iterator(textures_path("albedo"))) {
        addFromPath(entry);
    }

    for (const auto& entry : std::filesystem::directory_iterator(textures_path("normal"))) {
        addFromPath(entry);
    }

    initialized = true;
}

bool TextureManager::update() {
    bool textures_updated = false;
    const auto albedo_folder = std::filesystem::directory_entry(textures_path("albedo"));
    if (last_albedo_write_time != albedo_folder.last_write_time()) {
        last_albedo_write_time = albedo_folder.last_write_time();
        textures_updated = true;
        for (const auto& entry : std::filesystem::directory_iterator(textures_path("albedo"))) {
            const std::string texture_name = entry.path().stem().string();
            if ((write_times.find(texture_name) == write_times.end()) ||
                entry.last_write_time() > write_times[texture_name]) {
                addFromPath(entry);
            }
        }
    }
    return textures_updated;
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

VirtualTextureHandle TextureManager::getVirtual(const std::string& name) const {
    if (name_to_virtual.find(name) == name_to_virtual.end()) {
        LOG_ERROR("Failed to find virutal ID for texture {}", name);
        return 0;
    }
    return name_to_virtual.at(name);
}

TextureHandle TextureManager::get(VirtualTextureHandle virtual_handle) const {
    if (virtual_to_actual.find(virtual_handle) == virtual_to_actual.end()) {
        LOG_ERROR("The virtual texture ID '{}'", virtual_handle)
        return 0;
    }
    return virtual_to_actual.at(virtual_handle);
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
    texture_count++;
    textures[name] = texture;
    name_to_virtual[name] = texture_count;
    virtual_to_actual[texture_count] = texture;
}
