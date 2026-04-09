#pragma once

#include <memory>
#include <glm/glm.hpp>

class Shader;
class Camera;
class Model;

struct RenderStats {
    int   drawCalls  = 0;
    int   triangles  = 0;
    float frameTime  = 0.0f;
    float fps        = 0.0f;
};

struct LightSettings {
    glm::vec3 direction  = glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f));
    glm::vec3 ambient    = {0.15f, 0.15f, 0.18f};
    glm::vec3 diffuse    = {0.9f,  0.88f, 0.85f};
    glm::vec3 specular   = {1.0f,  1.0f,  1.0f};
};

enum class ShadingMode {
    Phong,
    Flat,
    Normal,
    Depth,
    Wireframe,
    UV,
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height);
    void resize(int width, int height);

    void beginFrame();
    void endFrame();

    void draw(const Model& model, const Camera& camera);

    void setShadingMode(ShadingMode mode) { m_shadingMode = mode; }
    void setWireframe(bool on)            { m_wireframe = on; }
    void setShowNormals(bool on)          { m_showNormals = on; }
    void setBackgroundColor(const glm::vec3& c) { m_bgColor = c; }
    void setLight(const LightSettings& l)       { m_light = l; }

    ShadingMode       shadingMode() const { return m_shadingMode; }
    bool              wireframe()   const { return m_wireframe; }
    bool              showNormals() const { return m_showNormals; }
    const LightSettings& light()   const { return m_light; }
    const RenderStats&   stats()   const { return m_stats; }

    void drawGrid(const Camera& camera, float size = 10.0f, int divisions = 20);
    void drawSkyGradient();

private:
    void loadShaders();
    void setupGrid();
    void setupFullscreenQuad();

    std::unique_ptr<Shader> m_phongShader;
    std::unique_ptr<Shader> m_flatShader;
    std::unique_ptr<Shader> m_normalShader;
    std::unique_ptr<Shader> m_wireShader;
    std::unique_ptr<Shader> m_gridShader;
    std::unique_ptr<Shader> m_normalVizShader;
    std::unique_ptr<Shader> m_skyShader;

    ShadingMode  m_shadingMode = ShadingMode::Phong;
    bool         m_wireframe   = false;
    bool         m_showNormals = false;
    glm::vec3    m_bgColor     = {0.12f, 0.12f, 0.14f};
    LightSettings m_light;
    RenderStats   m_stats;

    unsigned int m_gridVAO = 0;
    unsigned int m_gridVBO = 0;
    int          m_gridVertCount = 0;

    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    int m_width  = 0;
    int m_height = 0;
};
