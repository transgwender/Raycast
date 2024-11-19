#pragma once
#include "common.hpp"

#include <unordered_map>

#include <filesystem>

typedef GLuint TextureHandle;

/**
 * A `VirtualTextureHandle` is an abstraction over OpenGL texture IDs that
 * guarantee that the IDs are sequential and start at 1. This is not
 * guaranteed from actual OpenGL texture IDs as they are driver implementation dependent.
 * This virtual ID can then be used to get the actual OpenGL ID at any point.
 */
typedef size_t VirtualTextureHandle;

/**
 * Handles all the OpenGL textures for the renderer during the lifetime of the program.
 */
class TextureManager {
    bool initialized = false;

    std::unordered_map<std::string, TextureHandle> textures;

    std::unordered_map<std::string, VirtualTextureHandle> name_to_virtual;
    std::unordered_map<VirtualTextureHandle, TextureHandle> virtual_to_actual;

    std::unordered_map<std::string, std::filesystem::file_time_type> write_times;
    std::filesystem::file_time_type last_albedo_write_time;
    std::filesystem::file_time_type last_normal_write_time;

    size_t texture_count = 0;

    void addFromPath(const std::filesystem::directory_entry& entry);

  public:
    /**
     * Initialize the texture system. This loads all textures in the textures
     * folder.
     *
     * Using this class without calling `init` first will lead to undefined behaviour.
     */
    void init();

    [[nodiscard]] TextureHandle get(const std::string& name) const;

    [[nodiscard]] TextureHandle get(VirtualTextureHandle virtual_handle) const;

    /**
     * Given a texture name, returns the corresponding normal map.
     * If no map exists, returns a default flat normal map.
     */
    [[nodiscard]] TextureHandle getNormal(const std::string& name) const;

    /**
     * Get the `VirtualTextureHandle` for a texture given its name.
     * @param name The name of the texture
     * @return the virtual ID
     */
    [[nodiscard]] VirtualTextureHandle getVirtual(const std::string& name) const;

    /**
     * Manually add an internal texture with an associated name.
     * @param name The texture name
     * @param texture Renderer texture ID
     */
    void add(const std::string& name, const TextureHandle& texture);

    /**
     * Update any textures that have updated on disk since the last time the texture manager
     * was initialized/updated.
     * @return whether any textures were updated
     */
    bool update();
};