#pragma once
#include "common.hpp"
#include <unordered_map>

#include <filesystem>

typedef GLuint TextureHandle;

class TextureManager {
    std::unordered_map<std::string, TextureHandle> textures;

    void addFromPath(const std::filesystem::path& path);

  public:
    /**
     * Initialize the texture system. This loads all textures in the textures
     * folder.
     */
    void init();

    [[nodiscard]] TextureHandle get(const std::string& name) const;

    /**
     * Given a texture name, returns the corresponding normal map.
     * If no map exists, returns a default flat normal map.
     */
    [[nodiscard]] TextureHandle getNormal(const std::string& name) const;
};