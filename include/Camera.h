#pragma once

#include <glm/glm.hpp>

enum class CameraMode { Orbit, Fly, Pan };

class Camera {
public:
    Camera(float fov = 45.0f, float aspect = 16.0f / 9.0f,
           float near = 0.01f, float far = 1000.0f);

    glm::mat4 view()       const;
    glm::mat4 projection() const;
    glm::mat4 viewProj()   const { return projection() * view(); }

    void orbit(float deltaX, float deltaY);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);
    void setTarget(const glm::vec3& target);
    void fitToSphere(const glm::vec3& center, float radius);

    void moveForward(float speed);
    void moveRight(float speed);
    void moveUp(float speed);
    void rotate(float yaw, float pitch);

    void        setMode(CameraMode mode) { m_mode = mode; }
    CameraMode  mode()                   const { return m_mode; }
    glm::vec3   position()               const { return m_position; }
    glm::vec3   target()                 const { return m_target; }
    float       fov()                    const { return m_fov; }
    void        setAspect(float aspect)  { m_aspect = aspect; }
    void        setFov(float fov)        { m_fov = fov; }

    void reset();

private:
    void updateFromSpherical();

    glm::vec3 m_position = {0.0f, 0.0f, 5.0f};
    glm::vec3 m_target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_up       = {0.0f, 1.0f, 0.0f};

    float m_theta  = 0.0f;
    float m_phi    = glm::radians(30.0f);
    float m_radius = 5.0f;

    float m_yaw   = -90.0f;
    float m_pitch =   0.0f;

    float m_fov    = 45.0f;
    float m_aspect = 16.0f / 9.0f;
    float m_near   = 0.01f;
    float m_far    = 1000.0f;

    CameraMode m_mode = CameraMode::Orbit;
};
