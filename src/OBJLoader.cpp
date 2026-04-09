#include "OBJLoader.h"
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

namespace OBJLoader {

static std::unordered_map<std::string, Material>
loadMTL(const std::string& path) {
    std::unordered_map<std::string, Material> mats;
    std::ifstream f(path);
    if (!f.is_open()) return mats;

    std::string line, current;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tok; ss >> tok;

        if (tok == "newmtl") {
            ss >> current;
            mats[current].name = current;
        } else if (tok == "Ka" && !current.empty()) {
            ss >> mats[current].ambient.r
               >> mats[current].ambient.g
               >> mats[current].ambient.b;
        } else if (tok == "Kd" && !current.empty()) {
            ss >> mats[current].diffuse.r
               >> mats[current].diffuse.g
               >> mats[current].diffuse.b;
        } else if (tok == "Ks" && !current.empty()) {
            ss >> mats[current].specular.r
               >> mats[current].specular.g
               >> mats[current].specular.b;
        } else if (tok == "Ns" && !current.empty()) {
            ss >> mats[current].shininess;
        }
    }
    return mats;
}

LoadResult load(const std::string& filepath) {
    LoadResult result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        result.error = "Cannot open file: " + filepath;
        return result;
    }

    fs::path dir = fs::path(filepath).parent_path();

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    struct Group {
        std::string name;
        std::string matName;
        std::vector<std::array<int,9>> faces;
    };

    std::vector<Group>  groups;
    Group*              current = nullptr;
    std::unordered_map<std::string, Material> materials;

    auto ensureGroup = [&]() {
        if (!current) {
            groups.push_back({"default", "", {}});
            current = &groups.back();
        }
    };

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::istringstream ss(line);
        std::string tok; ss >> tok;

        if (tok == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (tok == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        } else if (tok == "vt") {
            glm::vec2 t;
            ss >> t.x >> t.y;
            texcoords.push_back(t);
        } else if (tok == "g" || tok == "o") {
            std::string name; ss >> name;
            groups.push_back({name, current ? current->matName : "", {}});
            current = &groups.back();
        } else if (tok == "usemtl") {
            ensureGroup();
            ss >> current->matName;
        } else if (tok == "mtllib") {
            std::string mtlFile; ss >> mtlFile;
            auto mtlPath = (dir / mtlFile).string();
            auto loaded = loadMTL(mtlPath);
            materials.insert(loaded.begin(), loaded.end());
        } else if (tok == "f") {
            ensureGroup();

            std::vector<std::array<int,3>> fverts;
            std::string token;
            while (ss >> token) {
                std::array<int,3> indices = {0, 0, 0};

                size_t s1 = token.find('/');
                if (s1 == std::string::npos) {
                    indices[0] = std::stoi(token);
                } else {
                    indices[0] = std::stoi(token.substr(0, s1));
                    size_t s2 = token.find('/', s1 + 1);
                    if (s2 == std::string::npos) {
                        indices[1] = std::stoi(token.substr(s1 + 1));
                    } else {
                        if (s2 > s1 + 1)
                            indices[1] = std::stoi(token.substr(s1+1, s2-s1-1));
                        if (s2 + 1 < token.size())
                            indices[2] = std::stoi(token.substr(s2 + 1));
                    }
                }
                fverts.push_back(indices);
            }

            for (size_t i = 1; i + 1 < fverts.size(); i++) {
                current->faces.push_back({
                    fverts[0][0], fverts[0][1], fverts[0][2],
                    fverts[i][0], fverts[i][1], fverts[i][2],
                    fverts[i+1][0], fverts[i+1][1], fverts[i+1][2]
                });
            }
        }
    }

    if (groups.empty()) {
        result.error = "No geometry found in: " + filepath;
        return result;
    }

    for (auto& grp : groups) {
        if (grp.faces.empty()) continue;

        std::vector<Vertex> verts;
        std::vector<unsigned int> idx;
        std::unordered_map<std::string, unsigned int> cache;

        for (auto& face : grp.faces) {
            for (int c = 0; c < 3; c++) {
                int pi = face[c*3 + 0];
                int ti = face[c*3 + 1];
                int ni = face[c*3 + 2];

                std::string key = std::to_string(pi) + "/" +
                                  std::to_string(ti) + "/" +
                                  std::to_string(ni);
                auto it = cache.find(key);
                if (it != cache.end()) {
                    idx.push_back(it->second);
                    continue;
                }

                Vertex v{};
                if (pi > 0 && pi <= (int)positions.size())
                    v.position  = positions[pi - 1];
                else if (pi < 0 && -pi <= (int)positions.size())
                    v.position  = positions[positions.size() + pi];

                if (ti > 0 && ti <= (int)texcoords.size())
                    v.texCoords = texcoords[ti - 1];

                if (ni > 0 && ni <= (int)normals.size())
                    v.normal    = normals[ni - 1];

                unsigned int newIdx = (unsigned int)verts.size();
                verts.push_back(v);
                cache[key] = newIdx;
                idx.push_back(newIdx);
            }
        }

        Material mat;
        if (!grp.matName.empty()) {
            auto it = materials.find(grp.matName);
            if (it != materials.end()) mat = it->second;
        }

        Mesh mesh(std::move(verts), std::move(idx), mat);
        if (normals.empty()) mesh.computeNormals();
        mesh.computeTangents();
        result.meshes.push_back(std::move(mesh));
    }

    if (result.meshes.empty())
        result.error = "No valid meshes in: " + filepath;

    return result;
}

bool save(const std::vector<Mesh>& meshes, const std::string& filepath) {
    std::ofstream f(filepath);
    if (!f.is_open()) return false;

    f << "# meshforge obj export\n";
    f << "# meshes: " << meshes.size() << "\n\n";

    unsigned int posOffset = 1;
    for (size_t mi = 0; mi < meshes.size(); mi++) {
        const auto& mesh = meshes[mi];
        f << "g mesh_" << mi << "\n";

        for (const auto& v : mesh.vertices()) {
            f << "v "  << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
        }
        for (const auto& v : mesh.vertices()) {
            f << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
        }
        for (const auto& v : mesh.vertices()) {
            f << "vt " << v.texCoords.x << " " << v.texCoords.y << "\n";
        }

        const auto& idx = mesh.indices();
        for (size_t i = 0; i + 2 < idx.size(); i += 3) {
            auto a = idx[i]   + posOffset;
            auto b = idx[i+1] + posOffset;
            auto c = idx[i+2] + posOffset;
            f << "f " << a<<"/"<<a<<"/"<<a << " "
                      << b<<"/"<<b<<"/"<<b << " "
                      << c<<"/"<<c<<"/"<<c << "\n";
        }
        f << "\n";
        posOffset += (unsigned int)mesh.vertices().size();
    }

    std::cout << "[obj] Saved: " << filepath << "\n";
    return true;
}

}
