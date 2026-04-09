#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
    Transform() = default;

    void setPosition(const glm::vec3& pos)   { m_position = pos;   m_dirty = true; }
    void setRotation(const glm::quat& rot)   { m_rotation = rot;   m_dirty = true; }
    void setRotation(const glm::vec3& euler) ;
    void setScale   (const glm::vec3& scale) { m_scale    = scale; m_dirty = true; }
    void setScale   (float uniform)          { m_scale = glm::vec3(uniform); m_dirty = true; }

    void translate(const glm::vec3& delta);
    void rotate   (float angleDeg, const glm::vec3& axis);
    void scale    (float factor);

    const glm::vec3& position() const { return m_position; }
    const glm::quat& rotation() const { return m_rotation; }
    const glm::vec3& scale()    const { return m_scale; }
    glm::vec3        eulerDeg() const;

    const glm::mat4& matrix() const;
    glm::mat3        normalMatrix() const;

    void reset();

private:
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_scale    = {1.0f, 1.0f, 1.0f};

    mutable glm::mat4 m_matrix;
    mutable bool      m_dirty = true;
};
