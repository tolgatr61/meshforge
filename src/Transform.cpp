#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

void Transform::setRotation(const glm::vec3& euler) {
    m_rotation = glm::quat(glm::radians(euler));
    m_dirty = true;
}

void Transform::translate(const glm::vec3& delta) {
    m_position += delta;
    m_dirty = true;
}

void Transform::rotate(float angleDeg, const glm::vec3& axis) {
    m_rotation = glm::angleAxis(glm::radians(angleDeg), glm::normalize(axis))
                 * m_rotation;
    m_dirty = true;
}

void Transform::scale(float f) {
    m_scale *= f;
    m_dirty = true;
}

const glm::mat4& Transform::matrix() const {
    if (m_dirty) {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
        glm::mat4 R = glm::mat4_cast(m_rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), m_scale);
        m_matrix = T * R * S;
        m_dirty = false;
    }
    return m_matrix;
}

glm::mat3 Transform::normalMatrix() const {
    return glm::mat3(glm::transpose(glm::inverse(matrix())));
}

glm::vec3 Transform::eulerDeg() const {
    return glm::degrees(glm::eulerAngles(m_rotation));
}

void Transform::reset() {
    m_position = {0.0f, 0.0f, 0.0f};
    m_rotation = glm::identity<glm::quat>();
    m_scale    = {1.0f, 1.0f, 1.0f};
    m_dirty    = true;
}
