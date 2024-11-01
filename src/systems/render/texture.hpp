#pragma once
#include "common.hpp"

#include <unordered_map>

#include <filesystem>

typedef GLuint TextureHandle;

/**
 * Handles all the OpenGL textures for the renderer during the lifetime of the program.
 */
class TextureManager {
    bool initialized = false;
    std::unordered_map<std::string, TextureHandle> textures;

    void addFromPath(const std::filesystem::path& path);

  public:
    /**
     * Initialize the texture system. This loads all textures in the textures
     * folder.
     *
     * Using this class without calling `init` first will lead to undefined behaviour.
     */
    void init();

    [[nodiscard]] TextureHandle get(const std::string& name) const;

    /**
     * Given a texture name, returns the corresponding normal map.
     * If no map exists, returns a default flat normal map.
     */
    [[nodiscard]] TextureHandle getNormal(const std::string& name) const;

    /**
     * Manually add an internal texture with an associated name.
     * @param name The texture name
     * @param texture Renderer texture ID
     */
    void add(const std::string& name, const TextureHandle& texture);
};