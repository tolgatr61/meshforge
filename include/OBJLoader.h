#pragma once

#include <string>
#include <vector>
#include "Mesh.h"

namespace OBJLoader {

    struct LoadResult {
        std::vector<Mesh> meshes;
        std::string       error;
        bool              ok() const { return error.empty(); }
    };

    LoadResult load(const std::string& filepath);

    bool save(const std::vector<Mesh>& meshes, const std::string& filepath);

}
