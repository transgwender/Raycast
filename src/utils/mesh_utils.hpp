#pragma once

#include "common.hpp"
#include "mesh_utils.hpp"
#include "stages/mesh.hpp"

class MeshUtils {
public:
    static bool loadFromOBJFile(const std::string& obj_path, std::vector<ColoredVertex>& out_vertices,
                         std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
};
