#pragma once

#include <string>
#include <memory>
#include <vector>

struct GLFWwindow;
class Renderer;
class Camera;
class Model;
class InputHandler;
class UI;

struct AppConfig {
    std::string title   = "MeshForge";
    int         width   = 1280;
    int         height  = 720;
    bool        vsync   = true;
    bool        msaa    = true;
    int         samples = 4;
};

class Application {
public:
    explicit Application(AppConfig config = {});
    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    void run();
    void shutdown();

    bool loadModel(const std::string& path);
    void loadBuiltinMesh(const std::string& name);
    void resetScene();
    void toggleWireframe();
    void toggleNormals();

    GLFWwindow*  getWindow()   const { return m_window; }
    AppConfig&   getConfig()         { return m_config; }
    Renderer*    getRenderer() const { return m_renderer.get(); }
    Camera*      getCamera()   const { return m_camera.get(); }

    std::vector<std::unique_ptr<Model>>& getModels() { return m_models; }

private:
    void processInput(float dt);
    void updateTitle();

    GLFWwindow*                         m_window   = nullptr;
    AppConfig                           m_config;
    std::unique_ptr<Renderer>           m_renderer;
    std::unique_ptr<Camera>             m_camera;
    std::unique_ptr<InputHandler>       m_input;
    std::unique_ptr<UI>                 m_ui;
    std::vector<std::unique_ptr<Model>> m_models;

    bool  m_running     = false;
    bool  m_wireframe   = false;
    bool  m_showNormals = false;
    float m_lastTime    = 0.0f;
};
