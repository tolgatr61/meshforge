#include <GL/glew.h>
#include "Mesh.h"
#include "Shader.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

float AABB::radius() const {
    auto e = extents();
    return std::sqrt(e.x*e.x + e.y*e.y + e.z*e.z);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           Material material)
    : m_vertices(std::move(vertices))
    , m_indices (std::move(indices))
    , m_material(std::move(material))
{
    computeBounds();
    setupGL();
}

Mesh::Mesh(Mesh&& o) noexcept
    : m_vertices(std::move(o.m_vertices))
    , m_indices (std::move(o.m_indices))
    , m_material(std::move(o.m_material))
    , m_bounds  (o.m_bounds)
    , m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo)
{
    o.m_vao = o.m_vbo = o.m_ebo = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);
        m_vertices = std::move(o.m_vertices);
        m_indices  = std::move(o.m_indices);
        m_material = std::move(o.m_material);
        m_bounds   = o.m_bounds;
        m_vao = o.m_vao; m_vbo = o.m_vbo; m_ebo = o.m_ebo;
        o.m_vao = o.m_vbo = o.m_ebo = 0;
    }
    return *this;
}

Mesh::~Mesh() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

void Mesh::setupGL() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertices.size() * sizeof(Vertex),
                 m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indices.size() * sizeof(unsigned int),
                 m_indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}

void Mesh::draw(const Shader& shader) const {
    if (!m_vao) return;
    shader.setVec3 ("material.ambient",  m_material.ambient);
    shader.setVec3 ("material.diffuse",  m_material.diffuse);
    shader.setVec3 ("material.specular", m_material.specular);
    shader.setFloat("material.shininess",m_material.shininess);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::drawWireframe(const Shader& shader) const {
    if (!m_vao) return;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::computeBounds() {
    m_bounds = {};
    for (const auto& v : m_vertices) {
        m_bounds.min = glm::min(m_bounds.min, v.position);
        m_bounds.max = glm::max(m_bounds.max, v.position);
    }
}

void Mesh::computeNormals() {

    for (auto& v : m_vertices) v.normal = glm::vec3(0.0f);

    for (size_t i = 0; i + 2 < m_indices.size(); i += 3) {
        auto& v0 = m_vertices[m_indices[i]];
        auto& v1 = m_vertices[m_indices[i+1]];
        auto& v2 = m_vertices[m_indices[i+2]];
        auto n = glm::cross(v1.position - v0.position,
                            v2.position - v0.position);
        v0.normal += n; v1.normal += n; v2.normal += n;
    }

    for (auto& v : m_vertices)
        if (glm::length(v.normal) > 1e-6f)
            v.normal = glm::normalize(v.normal);

    if (m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(Vertex),
                     m_vertices.data(), GL_STATIC_DRAW);
    }
}

void Mesh::flipNormals() {
    for (auto& v : m_vertices) v.normal = -v.normal;
    if (m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(Vertex),
                     m_vertices.data(), GL_STATIC_DRAW);
    }
}

void Mesh::center() {
    auto c = m_bounds.center();
    for (auto& v : m_vertices) v.position -= c;
    computeBounds();
    if (m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(Vertex),
                     m_vertices.data(), GL_STATIC_DRAW);
    }
}

void Mesh::normalize() {
    center();
    float r = m_bounds.radius();
    if (r < 1e-6f) return;
    for (auto& v : m_vertices) v.position /= r;
    computeBounds();
    if (m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(Vertex),
                     m_vertices.data(), GL_STATIC_DRAW);
    }
}

void Mesh::computeTangents() {
    for (auto& v : m_vertices) {
        v.tangent   = glm::vec3(0.0f);
        v.bitangent = glm::vec3(0.0f);
    }
    for (size_t i = 0; i + 2 < m_indices.size(); i += 3) {
        auto& v0 = m_vertices[m_indices[i]];
        auto& v1 = m_vertices[m_indices[i+1]];
        auto& v2 = m_vertices[m_indices[i+2]];

        glm::vec3 e1 = v1.position - v0.position;
        glm::vec3 e2 = v2.position - v0.position;
        glm::vec2 d1 = v1.texCoords - v0.texCoords;
        glm::vec2 d2 = v2.texCoords - v0.texCoords;

        float f = 1.0f / (d1.x * d2.y - d2.x * d1.y + 1e-9f);
        glm::vec3 t = f * (d2.y * e1 - d1.y * e2);
        glm::vec3 b = f * (-d2.x * e1 + d1.x * e2);

        v0.tangent += t; v1.tangent += t; v2.tangent += t;
        v0.bitangent += b; v1.bitangent += b; v2.bitangent += b;
    }
    for (auto& v : m_vertices) {
        v.tangent   = glm::normalize(v.tangent);
        v.bitangent = glm::normalize(v.bitangent);
    }
    if (m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices.size() * sizeof(Vertex),
                     m_vertices.data(), GL_STATIC_DRAW);
    }
}
