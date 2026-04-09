#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <chrono>

Renderer::Renderer() = default;
Renderer::~Renderer() {
    if (m_gridVAO) glDeleteVertexArrays(1, &m_gridVAO);
    if (m_gridVBO) glDeleteBuffers(1, &m_gridVBO);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
}

bool Renderer::init(int width, int height) {
    m_width  = width;
    m_height = height;

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "[render] GLEW init failed: "
                  << glewGetErrorString(err) << "\n";
        return false;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";
    std::cout << "GPU: "    << glGetString(GL_RENDERER) << "\n";

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    loadShaders();
    setupGrid();
    setupFullscreenQuad();
    return true;
}

void Renderer::resize(int w, int h) {
    m_width = w; m_height = h;
    glViewport(0, 0, w, h);
}

void Renderer::loadShaders() {

    const char* phongVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in vec3 aTangent;

uniform mat4 model;
uniform mat4 viewProj;
uniform mat3 normalMatrix;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragUV;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    fragPos       = worldPos.xyz;
    fragNormal    = normalize(normalMatrix * aNormal);
    fragUV        = aUV;
    gl_Position   = viewProj * worldPos;
}
)";

    const char* phongFS = R"(
#version 330 core
in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragUV;

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};
struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light     light;
uniform vec3      camPos;
uniform int       shadingMode;

out vec4 fragColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(-light.direction);
    vec3 V = normalize(camPos - fragPos);
    vec3 R = reflect(-L, N);

    if (shadingMode == 2) {

        fragColor = vec4(N * 0.5 + 0.5, 1.0);
        return;
    }
    if (shadingMode == 3) {

        float d = gl_FragCoord.z;
        fragColor = vec4(vec3(d), 1.0);
        return;
    }
    if (shadingMode == 4) {

        vec2 checker = floor(fragUV * 8.0);
        float c = mod(checker.x + checker.y, 2.0);
        fragColor = vec4(vec3(c * 0.6 + 0.2), 1.0);
        return;
    }

    vec3 ambient  = light.ambient  * material.ambient;
    float diff    = max(dot(N, L), 0.0);
    vec3 diffuse  = light.diffuse  * material.diffuse * diff;
    float spec    = pow(max(dot(V, R), 0.0), material.shininess);
    vec3 specular = light.specular * material.specular * spec;

    float rim = 1.0 - max(dot(V, N), 0.0);
    rim = pow(rim, 3.0) * 0.15;
    vec3 rimColor = vec3(0.4, 0.6, 1.0) * rim;

    vec3 col = ambient + diffuse + specular + rimColor;

    col = pow(col, vec3(1.0/2.2));
    fragColor = vec4(col, 1.0);
}
)";

    const char* wireVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model;
uniform mat4 viewProj;
void main() {
    gl_Position = viewProj * model * vec4(aPos, 1.0);
}
)";
    const char* wireFS = R"(
#version 330 core
uniform vec4 color;
out vec4 fragColor;
void main() { fragColor = color; }
)";

    const char* gridVS = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 viewProj;
uniform float alpha;
out float vAlpha;
void main() {
    vAlpha = alpha;
    gl_Position = viewProj * vec4(aPos, 1.0);
}
)";
    const char* gridFS = R"(
#version 330 core
in float vAlpha;
uniform vec3 color;
out vec4 fragColor;
void main() { fragColor = vec4(color, vAlpha); }
)";

    const char* skyVS = R"(
#version 330 core
layout(location=0) in vec2 aPos;
out vec2 vUV;
void main() {
    vUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.999, 1.0);
}
)";
    const char* skyFS = R"(
#version 330 core
in vec2 vUV;
uniform vec3 topColor;
uniform vec3 botColor;
out vec4 fragColor;
void main() {
    vec3 c = mix(botColor, topColor, vUV.y);
    fragColor = vec4(c, 1.0);
}
)";

    m_phongShader  = std::make_unique<Shader>();
    m_wireShader   = std::make_unique<Shader>();
    m_gridShader   = std::make_unique<Shader>();
    m_skyShader    = std::make_unique<Shader>();

    m_phongShader->loadFromSource(phongVS, phongFS);
    m_wireShader ->loadFromSource(wireVS,  wireFS);
    m_gridShader ->loadFromSource(gridVS,  gridFS);
    m_skyShader  ->loadFromSource(skyVS,   skyFS);
}

void Renderer::setupGrid() {
    std::vector<float> lines;
    int   divs = 20;
    float size = 10.0f;
    float step = size * 2.0f / divs;

    for (int i = 0; i <= divs; i++) {
        float p = -size + i * step;

        lines.insert(lines.end(), {p, 0.0f, -size,  p, 0.0f, size});

        lines.insert(lines.end(), {-size, 0.0f, p,  size, 0.0f, p});
    }
    m_gridVertCount = (int)lines.size() / 3;

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float),
                 lines.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

void Renderer::setupFullscreenQuad() {
    float verts[] = {-1,-1,  1,-1,  1,1,  -1,-1,  1,1,  -1,1};
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

static auto s_frameStart = std::chrono::high_resolution_clock::now();

void Renderer::beginFrame() {
    s_frameStart = std::chrono::high_resolution_clock::now();
    m_stats.drawCalls = 0;
    m_stats.triangles = 0;

    glClearColor(m_bgColor.r, m_bgColor.g, m_bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    auto now  = std::chrono::high_resolution_clock::now();
    float ms  = std::chrono::duration<float, std::milli>(now - s_frameStart).count();
    m_stats.frameTime = ms;
    m_stats.fps       = 1000.0f / (ms + 0.001f);
}

void Renderer::drawSkyGradient() {
    glDepthFunc(GL_ALWAYS);
    m_skyShader->use();
    m_skyShader->setVec3("topColor", {0.08f, 0.10f, 0.16f});
    m_skyShader->setVec3("botColor", {0.05f, 0.06f, 0.09f});
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDepthFunc(GL_LEQUAL);
}

void Renderer::drawGrid(const Camera& camera, float , int ) {
    m_gridShader->use();
    m_gridShader->setMat4("viewProj", camera.viewProj());
    m_gridShader->setVec3("color",    {0.45f, 0.48f, 0.60f});
    m_gridShader->setFloat("alpha",   0.55f);
    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertCount);
    glBindVertexArray(0);
    m_stats.drawCalls++;
}

void Renderer::draw(const Model& model, const Camera& camera) {
    if (!model.isValid()) return;

    auto vp = camera.viewProj();

    Shader* shader = m_phongShader.get();

    shader->use();
    shader->setMat4("viewProj",     vp);
    shader->setVec3("camPos",       camera.position());
    shader->setInt ("shadingMode",  (int)m_shadingMode);

    shader->setVec3("light.direction", m_light.direction);
    shader->setVec3("light.ambient",   m_light.ambient);
    shader->setVec3("light.diffuse",   m_light.diffuse);
    shader->setVec3("light.specular",  m_light.specular);

    model.draw(*shader);
    m_stats.drawCalls++;
    m_stats.triangles += (int)model.triCount();

    if (m_wireframe) {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        m_wireShader->use();
        m_wireShader->setMat4("viewProj", vp);
        m_wireShader->setVec4("color", {0.8f, 0.9f, 1.0f, 0.5f});
        model.drawWireframe(*m_wireShader);
        glDisable(GL_POLYGON_OFFSET_LINE);
        m_stats.drawCalls++;
    }

    if (m_showNormals) {
        m_wireShader->use();
        m_wireShader->setMat4("viewProj", vp);
        m_wireShader->setVec4("color", {0.2f, 1.0f, 0.5f, 0.9f});

    }
}
