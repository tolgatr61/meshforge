#include "Model.h"
#include "OBJLoader.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Model::Model(const std::string& name) : m_name(name) {}

bool Model::loadOBJ(const std::string& path) {
    auto result = OBJLoader::load(path);
    if (!result.ok()) {
        std::cerr << "err: " << result.error << "\n";
        return false;
    }
    m_filepath = path;
    m_meshes   = std::move(result.meshes);
    centerAndNormalize();
    std::cout << "loaded '" << m_name << "': "
              << meshCount() << " mesh(es), "
              << vertCount() << " verts, "
              << triCount()  << " tris\n";
    return true;
}

void Model::addMesh(Mesh mesh) {
    m_meshes.push_back(std::move(mesh));
}

void Model::clearMeshes() {
    m_meshes.clear();
}

void Model::draw(const Shader& shader) const {
    shader.setMat4("model",       m_transform.matrix());
    shader.setMat3("normalMatrix",m_transform.normalMatrix());
    for (const auto& mesh : m_meshes)
        mesh.draw(shader);
}

void Model::drawWireframe(const Shader& shader) const {
    shader.setMat4("model", m_transform.matrix());
    for (const auto& mesh : m_meshes)
        mesh.drawWireframe(shader);
}

void Model::drawNormals(const Shader& shader, float ) const {
    shader.setMat4("model", m_transform.matrix());
    for (const auto& mesh : m_meshes)
        mesh.draw(shader);
}

AABB Model::worldAABB() const {
    AABB result;
    result.min = glm::vec3( 1e9f);
    result.max = glm::vec3(-1e9f);
    for (const auto& mesh : m_meshes) {
        auto& b = mesh.bounds();

        auto  M = m_transform.matrix();
        for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
        for (int z = 0; z < 2; z++) {
            glm::vec3 corner = {
                x ? b.max.x : b.min.x,
                y ? b.max.y : b.min.y,
                z ? b.max.z : b.min.z
            };
            glm::vec3 wc = glm::vec3(M * glm::vec4(corner, 1.0f));
            result.min = glm::min(result.min, wc);
            result.max = glm::max(result.max, wc);
        }
    }
    return result;
}

size_t Model::triCount() const {
    size_t t = 0;
    for (const auto& m : m_meshes) t += m.triCount();
    return t;
}

size_t Model::vertCount() const {
    size_t v = 0;
    for (const auto& m : m_meshes) v += m.vertCount();
    return v;
}

void Model::centerAndNormalize() {

    AABB aabb;
    aabb.min = glm::vec3( 1e9f);
    aabb.max = glm::vec3(-1e9f);
    for (auto& mesh : m_meshes) {
        aabb.min = glm::min(aabb.min, mesh.bounds().min);
        aabb.max = glm::max(aabb.max, mesh.bounds().max);
    }

    glm::vec3 center = aabb.center();
    float     radius = aabb.radius();
    if (radius < 1e-6f) return;

    m_transform.setPosition(-center / radius);
    m_transform.setScale(1.0f / radius);
}
