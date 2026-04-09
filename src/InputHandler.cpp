#include <GLFW/glfw3.h>
#include "InputHandler.h"
#include "Camera.h"
#include <iostream>

static InputHandler* g_instance = nullptr;

InputHandler::InputHandler(GLFWwindow* window) : m_window(window) {
    g_instance = this;
    glfwSetKeyCallback            (window, keyCallback);
    glfwSetMouseButtonCallback    (window, mouseButtonCallback);
    glfwSetCursorPosCallback      (window, cursorPosCallback);
    glfwSetScrollCallback         (window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    m_mousePos   = {(float)mx, (float)my};
    m_mousePrev2 = m_mousePos;
}

void InputHandler::update() {
    m_keysPrev    = m_keysCurrent;
    std::copy(m_mouseCurrent, m_mouseCurrent + 3, m_mousePrev);
    m_mouseDelta  = m_mousePos - m_mousePrev2;
    m_mousePrev2  = m_mousePos;
    m_scrollDelta = m_scrollAccum;
    m_scrollAccum = 0.0f;
}

bool InputHandler::isKeyDown(int key) const {
    auto it = m_keysCurrent.find(key);
    return it != m_keysCurrent.end() && it->second;
}

bool InputHandler::isKeyPressed(int key) const {
    bool cur  = isKeyDown(key);
    auto it   = m_keysPrev.find(key);
    bool prev = it != m_keysPrev.end() && it->second;
    return cur && !prev;
}

bool InputHandler::isKeyReleased(int key) const {
    bool cur  = isKeyDown(key);
    auto it   = m_keysPrev.find(key);
    bool prev = it != m_keysPrev.end() && it->second;
    return !cur && prev;
}

bool InputHandler::isMouseDown(MouseButton btn) const {
    return m_mouseCurrent[(int)btn];
}
bool InputHandler::isMousePressed(MouseButton btn) const {
    return  m_mouseCurrent[(int)btn] && !m_mousePrev[(int)btn];
}
bool InputHandler::isMouseReleased(MouseButton btn) const {
    return !m_mouseCurrent[(int)btn] &&  m_mousePrev[(int)btn];
}

void InputHandler::applyOrbitCamera(Camera& cam, float sens,
                                     float panSens, float zoomSens) {
    bool alt = isKeyDown(GLFW_KEY_LEFT_ALT) || isKeyDown(GLFW_KEY_RIGHT_ALT);

    if (isMouseDown(MouseButton::Left) && !alt) {
        cam.orbit(m_mouseDelta.x * sens, m_mouseDelta.y * sens);
    }
    if (isMouseDown(MouseButton::Middle) ||
        (isMouseDown(MouseButton::Left) && alt)) {
        cam.pan(m_mouseDelta.x * panSens, m_mouseDelta.y * panSens);
    }
    if (m_scrollDelta != 0.0f) {
        cam.zoom(m_scrollDelta * zoomSens);
    }
}

void InputHandler::keyCallback(GLFWwindow*, int key, int, int action, int) {
    if (!g_instance) return;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        g_instance->m_keysCurrent[key] = true;
    else if (action == GLFW_RELEASE)
        g_instance->m_keysCurrent[key] = false;
}

void InputHandler::mouseButtonCallback(GLFWwindow*, int btn, int action, int) {
    if (!g_instance || btn < 0 || btn >= 3) return;
    g_instance->m_mouseCurrent[btn] = (action == GLFW_PRESS);
}

void InputHandler::cursorPosCallback(GLFWwindow*, double x, double y) {
    if (!g_instance) return;
    g_instance->m_mousePos = {(float)x, (float)y};
}

void InputHandler::scrollCallback(GLFWwindow*, double, double dy) {
    if (!g_instance) return;
    g_instance->m_scrollAccum += (float)dy;
}

void InputHandler::framebufferSizeCallback(GLFWwindow*, int w, int h) {
    if (!g_instance) return;
    if (g_instance->onResize) g_instance->onResize(w, h);
}
