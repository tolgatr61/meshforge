#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

static constexpr float kPi = 3.14159265358979f;

Camera::Camera(float fov, float aspect, float near, float far)
    : m_fov(fov), m_aspect(aspect), m_near(near), m_far(far)
{
    updateFromSpherical();
}

glm::mat4 Camera::view() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::projection() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

void Camera::orbit(float dx, float dy) {
    m_theta -= glm::radians(dx);
    m_phi   -= glm::radians(dy);
    m_phi = std::clamp(m_phi, -kPi * 0.49f, kPi * 0.49f);
    updateFromSpherical();
}

void Camera::pan(float dx, float dy) {

    auto forward = glm::normalize(m_target - m_position);
    auto right   = glm::normalize(glm::cross(forward, m_up));
    auto up      = glm::cross(right, forward);

    glm::vec3 delta = (-right * dx + up * dy) * m_radius;
    m_target   += delta;
    m_position += delta;
}

void Camera::zoom(float delta) {
    m_radius -= delta * m_radius * 0.1f;
    m_radius = std::clamp(m_radius, 0.05f, 500.0f);
    updateFromSpherical();
}

void Camera::setTarget(const glm::vec3& target) {
    auto diff  = m_position - m_target;
    m_target   = target;
    m_position = target + diff;
}

void Camera::fitToSphere(const glm::vec3& center, float radius) {
    m_target = center;
    float dist = radius / std::tan(glm::radians(m_fov * 0.5f)) * 1.2f;
    m_radius = dist;
    updateFromSpherical();
    m_near = radius * 0.01f;
    m_far  = dist + radius * 10.0f;
}

void Camera::updateFromSpherical() {
    m_position.x = m_target.x + m_radius * std::cos(m_phi) * std::cos(m_theta);
    m_position.y = m_target.y + m_radius * std::sin(m_phi);
    m_position.z = m_target.z + m_radius * std::cos(m_phi) * std::sin(m_theta);
}

void Camera::moveForward(float speed) {
    auto dir = glm::normalize(m_target - m_position);
    m_position += dir * speed;
    m_target   += dir * speed;
}

void Camera::moveRight(float speed) {
    auto dir   = glm::normalize(m_target - m_position);
    auto right = glm::normalize(glm::cross(dir, m_up));
    m_position += right * speed;
    m_target   += right * speed;
}

void Camera::moveUp(float speed) {
    m_position += m_up * speed;
    m_target   += m_up * speed;
}

void Camera::rotate(float yaw, float pitch) {

}

void Camera::reset() {
    m_theta  = 0.0f;
    m_phi    = glm::radians(20.0f);
    m_radius = 5.0f;
    m_target = {0.0f, 0.0f, 0.0f};
    updateFromSpherical();
}
