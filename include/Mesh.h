#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Material {
    glm::vec3 ambient  = {0.2f, 0.2f, 0.2f};
    glm::vec3 diffuse  = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float     shininess = 32.0f;
    std::string name;
};

struct AABB {
    glm::vec3 min{ 1e9f};
    glm::vec3 max{-1e9f};

    glm::vec3 center()  const { return (min + max) * 0.5f; }
    glm::vec3 extents() const { return (max - min) * 0.5f; }
    float     radius()  const;
};

class Shader;

class Mesh {
public:
    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
         Material material = {});
    ~Mesh();

    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;

    void draw(const Shader& shader) const;
    void drawWireframe(const Shader& shader) const;

    const std::vector<Vertex>&       vertices()  const { return m_vertices; }
    const std::vector<unsigned int>& indices()   const { return m_indices; }
    const Material&                  material()  const { return m_material; }
    const AABB&                      bounds()    const { return m_bounds; }
    size_t                           triCount()  const { return m_indices.size() / 3; }
    size_t                           vertCount() const { return m_vertices.size(); }

    void setMaterial(const Material& mat) { m_material = mat; }

    void computeNormals();
    void computeTangents();
    void center();
    void normalize();
    void flipNormals();

private:
    void setupGL();
    void computeBounds();

    std::vector<Vertex>       m_vertices;
    std::vector<unsigned int> m_indices;
    Material                  m_material;
    AABB                      m_bounds;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
};
