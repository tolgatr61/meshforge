#pragma once
#include <string>

struct GLFWwindow;
class Application;
struct RenderStats;

class UI {
public:
    explicit UI(Application* app);
    ~UI();

    bool init(GLFWwindow* window);
    void beginFrame();
    void render(const RenderStats& stats);
    void endFrame();
    void shutdown();

    void printHelp();
    void printModelInfo();

    bool wantsCaptureMouse()    const;
    bool wantsCaptureKeyboard() const;

private:
    void drawPanelScene();
    void drawPanelDisplay();
    void drawPanelCamera();
    void drawPanelLight();
    void drawStatsOverlay(const RenderStats& stats);
    void drawStatusBar();
    void applyDarkTheme();

    Application* m_app      = nullptr;
    float        m_fps      = 0.0f;
};
