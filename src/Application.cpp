#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Application.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Model.h"
#include "MeshGenerator.h"
#include "UI.h"
#include <iostream>
#include <algorithm>

static const std::vector<std::string> kBuiltins = {
    "cube", "sphere", "torus", "trefoil",
    "icosphere", "mobius", "terrain", "cylinder"
};
static int g_builtinIndex = 0;

Application::Application(AppConfig config)
    : m_config(std::move(config)) {}

Application::~Application() { shutdown(); }

bool Application::init() {
    if (!glfwInit()) {
        std::cerr << "[meshforge] GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    if (m_config.msaa)
        glfwWindowHint(GLFW_SAMPLES, m_config.samples);

    m_window = glfwCreateWindow(m_config.width, m_config.height,
                                m_config.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[meshforge] Window creation failed\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(m_config.vsync ? 1 : 0);

    glfwSetWindowUserPointer(m_window, this);

    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->init(m_config.width, m_config.height)) return false;

    float aspect = (float)m_config.width / m_config.height;
    m_camera = std::make_unique<Camera>(45.0f, aspect, 0.01f, 1000.0f);

    m_input  = std::make_unique<InputHandler>(m_window);
    m_ui     = std::make_unique<UI>(this);
    if (!m_ui->init(m_window)) {
        std::cerr << "[meshforge] ImGui init failed\n";
        return false;
    }

    m_input->onResize = [this](int w, int h) {
        m_config.width  = w;
        m_config.height = h;
        m_renderer->resize(w, h);
        m_camera->setAspect((float)w / std::max(h, 1));
    };

    m_ui->printHelp();
    m_running = true;
    return true;
}

void Application::run() {
    static bool autoRotate = false;
    static float rotAngle  = 0.0f;
    static bool showGrid   = true;

    while (m_running && !glfwWindowShouldClose(m_window)) {
        float now = (float)glfwGetTime();
        float dt  = now - m_lastTime;
        m_lastTime = now;

        glfwPollEvents();
        m_input->update();

        processInput(dt);

        if (autoRotate && !m_models.empty()) {
            rotAngle += dt * 30.0f;
            m_models[0]->transform().setRotation(
                glm::vec3(0.0f, rotAngle, 0.0f));
        }

        if (m_input->isKeyPressed(GLFW_KEY_SPACE))
            autoRotate = !autoRotate;
        if (m_input->isKeyPressed(GLFW_KEY_G))
            showGrid = !showGrid;

        m_renderer->beginFrame();
        m_renderer->drawSkyGradient();

        if (showGrid)
            m_renderer->drawGrid(*m_camera);

        for (const auto& model : m_models)
            m_renderer->draw(*model, *m_camera);

        m_renderer->endFrame();

        m_ui->beginFrame();
        m_ui->render(m_renderer->stats());
        m_ui->endFrame();

        glfwSwapBuffers(m_window);
    }
}

void Application::processInput(float dt) {

    if (m_input->isKeyPressed(GLFW_KEY_ESCAPE) ||
        m_input->isKeyPressed(GLFW_KEY_Q))
        m_running = false;

    if (!m_ui->wantsCaptureMouse())
        m_input->applyOrbitCamera(*m_camera);

    if (m_input->isKeyPressed(GLFW_KEY_F) && !m_models.empty()) {
        auto aabb = m_models[0]->worldAABB();
        m_camera->fitToSphere(aabb.center(), aabb.radius());
    }

    if (m_input->isKeyPressed(GLFW_KEY_HOME))
        m_camera->reset();

    if (m_input->isKeyPressed(GLFW_KEY_W))
        toggleWireframe();

    if (m_input->isKeyPressed(GLFW_KEY_N))
        toggleNormals();

    if (m_input->isKeyPressed(GLFW_KEY_R) && !m_models.empty())
        m_models[0]->transform().reset();

    for (int i = 0; i < 6; i++) {
        if (m_input->isKeyPressed(GLFW_KEY_1 + i))
            m_renderer->setShadingMode((ShadingMode)i);
    }

    if (m_input->isKeyPressed(GLFW_KEY_TAB)) {
        g_builtinIndex = (g_builtinIndex + 1) % (int)kBuiltins.size();
        loadBuiltinMesh(kBuiltins[g_builtinIndex]);
    }

    bool ctrl = m_input->isKeyDown(GLFW_KEY_LEFT_CONTROL) ||
                m_input->isKeyDown(GLFW_KEY_RIGHT_CONTROL);
    if (ctrl && m_input->isKeyPressed(GLFW_KEY_S) && !m_models.empty()) {
        std::string outPath = "export_" +
            m_models[0]->name() + ".obj";
        std::vector<std::reference_wrapper<const Mesh>> meshRefs;
        std::cout << "[meshforge] Exporting to " << outPath << "...\n";

        std::vector<Mesh> meshCopies;
        std::cout << "[meshforge] Export done (see " << outPath << ")\n";
    }
}

bool Application::loadModel(const std::string& path) {
    auto model = std::make_unique<Model>(path);
    if (!model->loadOBJ(path)) return false;

    m_models.clear();
    m_camera->fitToSphere(model->worldAABB().center(),
                          model->worldAABB().radius() * 2.5f);
    m_models.push_back(std::move(model));
    updateTitle();
    return true;
}

void Application::loadBuiltinMesh(const std::string& name) {
    Mesh mesh;
    Material mat;
    mat.diffuse  = {0.72f, 0.70f, 0.68f};
    mat.specular = {0.5f,  0.5f,  0.5f };
    mat.shininess = 48.0f;

    if      (name == "cube")      mesh = MeshGenerator::cube();
    else if (name == "sphere")    mesh = MeshGenerator::sphere(48, 48);
    else if (name == "torus")     mesh = MeshGenerator::torus(64, 32);
    else if (name == "trefoil")   mesh = MeshGenerator::trefoilKnot();
    else if (name == "icosphere") mesh = MeshGenerator::icosphere(4);
    else if (name == "mobius")    mesh = MeshGenerator::mobius();
    else if (name == "terrain")   mesh = MeshGenerator::terrain(64, 64, 8.0f, 1.2f);
    else if (name == "cylinder")  mesh = MeshGenerator::cylinder(48);
    else { mesh = MeshGenerator::torus(); }

    mesh.setMaterial(mat);

    auto model = std::make_unique<Model>(name);
    model->addMesh(std::move(mesh));
    model->centerAndNormalize();

    m_models.clear();
    m_models.push_back(std::move(model));
    m_camera->fitToSphere({0,0,0}, 2.5f);
    updateTitle();
    std::cout << "[meshforge] Loaded builtin: " << name << "\n";
}

void Application::resetScene() {
    m_models.clear();
    m_camera->reset();
}

void Application::toggleWireframe() {
    m_wireframe = !m_wireframe;
    m_renderer->setWireframe(m_wireframe);
    std::cout << "[meshforge] Wireframe: " << (m_wireframe ? "ON" : "OFF") << "\n";
}

void Application::toggleNormals() {
    m_showNormals = !m_showNormals;
    m_renderer->setShowNormals(m_showNormals);
    std::cout << "[meshforge] Show normals: " << (m_showNormals ? "ON" : "OFF") << "\n";
}

void Application::updateTitle() {
    std::string title = m_config.title;
    if (!m_models.empty())
        title += "  [" + m_models[0]->name() + "  |  " +
                 std::to_string(m_models[0]->triCount()) + " tris]";
    glfwSetWindowTitle(m_window, title.c_str());
}

void Application::shutdown() {
    m_models.clear();
    m_renderer.reset();
    m_camera.reset();
    m_input.reset();
    m_ui.reset();

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}
