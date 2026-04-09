#pragma once

#include "Mesh.h"
#include "Transform.h"
#include <vector>
#include <string>
#include <memory>

class Shader;

class Model {
public:
    explicit Model(const std::string& name = "unnamed");
    ~Model() = default;

    bool loadOBJ(const std::string& path);

    void draw(const Shader& shader) const;
    void drawWireframe(const Shader& shader) const;
    void drawNormals(const Shader& shader, float length = 0.1f) const;

    void addMesh(Mesh mesh);
    void clearMeshes();

    Transform&       transform()       { return m_transform; }
    const Transform& transform() const { return m_transform; }

    const std::string& name()      const { return m_name; }
    const std::string& filepath()  const { return m_filepath; }
    AABB               worldAABB() const;
    size_t             triCount()  const;
    size_t             vertCount() const;
    size_t             meshCount() const { return m_meshes.size(); }
    bool               isValid()   const { return !m_meshes.empty(); }

    void centerAndNormalize();

private:
    std::string        m_name;
    std::string        m_filepath;
    Transform          m_transform;
    std::vector<Mesh>  m_meshes;
};
