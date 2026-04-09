#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

struct GLFWwindow;
class Camera;

enum class MouseButton { Left, Middle, Right };

class InputHandler {
public:
    explicit InputHandler(GLFWwindow* window);

    void update();

    bool isKeyDown(int glfwKey)     const;
    bool isKeyPressed(int glfwKey)  const;
    bool isKeyReleased(int glfwKey) const;

    bool isMouseDown(MouseButton btn)     const;
    bool isMousePressed(MouseButton btn)  const;
    bool isMouseReleased(MouseButton btn) const;

    glm::vec2 mousePos()   const { return m_mousePos; }
    glm::vec2 mouseDelta() const { return m_mouseDelta; }
    float     scrollDelta()const { return m_scrollDelta; }

    void applyOrbitCamera(Camera& camera, float sensitivity = 0.4f,
                          float panSensitivity = 0.005f,
                          float zoomSensitivity = 0.3f);

    static void keyCallback     (GLFWwindow*, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow*, int, int, int);
    static void cursorPosCallback  (GLFWwindow*, double, double);
    static void scrollCallback     (GLFWwindow*, double, double);
    static void framebufferSizeCallback(GLFWwindow*, int, int);

    std::function<void(int,int)> onResize;

private:
    GLFWwindow* m_window;

    std::unordered_map<int, bool> m_keysCurrent;
    std::unordered_map<int, bool> m_keysPrev;

    bool m_mouseCurrent[3] = {};
    bool m_mousePrev[3]    = {};

    glm::vec2 m_mousePos   = {};
    glm::vec2 m_mousePrev2 = {};
    glm::vec2 m_mouseDelta = {};
    float     m_scrollDelta    = 0.0f;
    float     m_scrollAccum    = 0.0f;
};
