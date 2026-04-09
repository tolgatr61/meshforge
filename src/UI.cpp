#include "UI.h"
#include "Application.h"
#include "Renderer.h"
#include "Model.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <algorithm>
#include <deque>
#include <string>

static const char* kMeshNames[] = {
    "Trefoil Knot", "Torus", "Sphere", "Icosphere",
    "Mobius Strip", "Cube", "Cylinder", "Terrain"
};
static const char* kMeshKeys[] = {
    "trefoil", "torus", "sphere", "icosphere",
    "mobius", "cube", "cylinder", "terrain"
};
static const char* kShadingNames[] = {
    "Phong", "Flat", "Normals", "Depth", "UV"
};

static std::deque<std::string> g_history;

static void Tooltip(const char* text) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("%s", text);
}

UI::UI(Application* app) : m_app(app) {}
UI::~UI() { shutdown(); }

bool UI::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = "meshforge.ini";

    applyDarkTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    return true;
}

void UI::applyDarkTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 7.0f;
    s.FrameRounding     = 4.0f;
    s.GrabRounding      = 4.0f;
    s.PopupRounding     = 4.0f;
    s.ScrollbarRounding = 6.0f;
    s.WindowBorderSize  = 0.0f;
    s.FramePadding      = {8, 5};
    s.ItemSpacing       = {8, 6};
    s.WindowPadding     = {12, 12};

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]         = {0.09f, 0.10f, 0.12f, 0.94f};
    c[ImGuiCol_Header]           = {0.18f, 0.32f, 0.52f, 0.70f};
    c[ImGuiCol_HeaderHovered]    = {0.24f, 0.42f, 0.68f, 0.80f};
    c[ImGuiCol_HeaderActive]     = {0.28f, 0.48f, 0.78f, 1.00f};
    c[ImGuiCol_Button]           = {0.16f, 0.28f, 0.46f, 0.85f};
    c[ImGuiCol_ButtonHovered]    = {0.22f, 0.40f, 0.64f, 1.00f};
    c[ImGuiCol_ButtonActive]     = {0.14f, 0.24f, 0.40f, 1.00f};
    c[ImGuiCol_FrameBg]          = {0.14f, 0.16f, 0.19f, 1.00f};
    c[ImGuiCol_FrameBgHovered]   = {0.18f, 0.21f, 0.25f, 1.00f};
    c[ImGuiCol_SliderGrab]       = {0.32f, 0.52f, 0.84f, 1.00f};
    c[ImGuiCol_SliderGrabActive] = {0.42f, 0.65f, 1.00f, 1.00f};
    c[ImGuiCol_CheckMark]        = {0.42f, 0.72f, 1.00f, 1.00f};
    c[ImGuiCol_TitleBg]          = {0.07f, 0.08f, 0.10f, 1.00f};
    c[ImGuiCol_TitleBgActive]    = {0.10f, 0.18f, 0.30f, 1.00f};
    c[ImGuiCol_Separator]        = {0.22f, 0.26f, 0.32f, 1.00f};
    c[ImGuiCol_Tab]              = {0.10f, 0.16f, 0.26f, 1.00f};
    c[ImGuiCol_TabHovered]       = {0.22f, 0.38f, 0.60f, 1.00f};
    c[ImGuiCol_TabActive]        = {0.18f, 0.30f, 0.50f, 1.00f};
    c[ImGuiCol_PopupBg]          = {0.10f, 0.11f, 0.14f, 0.97f};
}

void UI::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool UI::wantsCaptureMouse()    const { return ImGui::GetIO().WantCaptureMouse;    }
bool UI::wantsCaptureKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

void UI::render(const RenderStats& stats) {
    float dt = ImGui::GetIO().DeltaTime;
    if (dt > 0.0f) {
        float raw = 1.0f / dt;
        m_fps = (m_fps < 1.0f) ? raw : m_fps * 0.92f + raw * 0.08f;
    }

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({275, 520}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.93f);

    ImGui::Begin("MeshForge", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("tabs")) {
        if (ImGui::BeginTabItem("Scene"))   { drawPanelScene();   ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Display")) { drawPanelDisplay(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Light"))   { drawPanelLight();   ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Camera"))  { drawPanelCamera();  ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::End();

    drawStatsOverlay(stats);
    drawStatusBar();
}

void UI::drawPanelScene() {
    ImGui::SeparatorText("Meshes");

    static int sel = 0;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginListBox("##meshes", {0, 128})) {
        for (int i = 0; i < IM_ARRAYSIZE(kMeshNames); i++) {
            bool active = (sel == i);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Header,        {0.18f,0.38f,0.65f,1.f});
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, {0.22f,0.44f,0.72f,1.f});
            }
            if (ImGui::Selectable(kMeshNames[i], active)) {
                sel = i;
                m_app->loadBuiltinMesh(kMeshKeys[i]);
                g_history.push_front(kMeshNames[i]);
                if (g_history.size() > 6) g_history.pop_back();
            }
            if (active) ImGui::PopStyleColor(2);
        }
        ImGui::EndListBox();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Transform");

    static float euler[3] = {0,0,0};
    static bool  autoSpin  = false;
    static float spinSpeed = 30.0f;

    bool rotChanged = false;
    ImGui::SetNextItemWidth(-1);
    rotChanged |= ImGui::SliderFloat("##rx", &euler[0], -180.f, 180.f, "X  %.0f deg");
    Tooltip("Rotation X");
    ImGui::SetNextItemWidth(-1);
    rotChanged |= ImGui::SliderFloat("##ry", &euler[1], -180.f, 180.f, "Y  %.0f deg");
    Tooltip("Rotation Y");
    ImGui::SetNextItemWidth(-1);
    rotChanged |= ImGui::SliderFloat("##rz", &euler[2], -180.f, 180.f, "Z  %.0f deg");
    Tooltip("Rotation Z");

    if (rotChanged && !m_app->getModels().empty()) {
        m_app->getModels()[0]->transform().setRotation(
            glm::vec3(euler[0], euler[1], euler[2])
        );
    }

    ImGui::Spacing();
    ImGui::Checkbox("Auto-spin", &autoSpin);
    Tooltip("Space pour toggle");
    if (autoSpin) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##spd", &spinSpeed, 5.f, 200.f, "%.0f /s");
    }
    if (autoSpin && !m_app->getModels().empty()) {
        euler[1] = fmodf(euler[1] + spinSpeed * ImGui::GetIO().DeltaTime, 360.f);
        m_app->getModels()[0]->transform().setRotation(
            glm::vec3(euler[0], euler[1], euler[2])
        );
    }

    if (ImGui::Button("Reset", {-1, 0})) {
        euler[0] = euler[1] = euler[2] = 0.f;
        if (!m_app->getModels().empty())
            m_app->getModels()[0]->transform().reset();
    }
    Tooltip("Remet la position, rotation et echelle a zero");

    if (!g_history.empty()) {
        ImGui::Spacing();
        ImGui::SeparatorText("Recent");
        for (auto& h : g_history) {
            ImGui::TextDisabled("  %s", h.c_str());
        }
    }

    if (!m_app->getModels().empty()) {
        auto* mdl = m_app->getModels()[0].get();
        ImGui::Spacing();
        ImGui::SeparatorText("Info");
        ImGui::Text("%zu tris   %zu verts", mdl->triCount(), mdl->vertCount());
    }
}

void UI::drawPanelDisplay() {
    auto* renderer = m_app->getRenderer();
    if (!renderer) return;

    ImGui::SeparatorText("Shading");

    int mode = (int)renderer->shadingMode();
    for (int i = 0; i < IM_ARRAYSIZE(kShadingNames); i++) {
        if (i > 0) ImGui::SameLine();
        bool active = (mode == i);
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, {0.18f,0.42f,0.78f,1.f});
        if (ImGui::Button(kShadingNames[i], {46, 0}))
            renderer->setShadingMode((ShadingMode)i);
        if (active) ImGui::PopStyleColor();
        Tooltip(kShadingNames[i]);
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Material");

    static float matColor[3] = {0.72f, 0.70f, 0.68f};
    static float roughness    = 0.5f;
    if (ImGui::ColorEdit3("Color", matColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel)) {
        if (!m_app->getModels().empty()) {
            Material mat;
            mat.diffuse  = {matColor[0], matColor[1], matColor[2]};
            mat.specular = {roughness * 0.5f + 0.5f, roughness * 0.5f + 0.5f, roughness * 0.5f + 0.5f};
            mat.shininess = (1.0f - roughness) * 128.0f + 4.0f;
        }
    }
    Tooltip("Couleur diffuse du materiau");
    ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.2f");
    Tooltip("0 = miroir  1 = mat");

    ImGui::Spacing();
    ImGui::SeparatorText("Overlays");

    bool wire = renderer->wireframe();
    if (ImGui::Checkbox("Wireframe  [W]", &wire))
        renderer->setWireframe(wire);

    bool normals = renderer->showNormals();
    if (ImGui::Checkbox("Normals  [N]", &normals))
        renderer->setShowNormals(normals);

    ImGui::Spacing();
    ImGui::SeparatorText("Background");

    static float bg[3] = {0.09f, 0.10f, 0.12f};
    if (ImGui::ColorEdit3("##bg", bg, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel))
        renderer->setBackgroundColor({bg[0], bg[1], bg[2]});
    ImGui::SameLine();
    ImGui::TextUnformatted("Background");
}

void UI::drawPanelLight() {
    auto* renderer = m_app->getRenderer();
    if (!renderer) return;

    LightSettings light = renderer->light();
    bool changed = false;

    ImGui::SeparatorText("Presets");

    struct Preset { const char* name; glm::vec3 dir, amb, dif, spe; };
    static const Preset presets[] = {
        {"Day",     {-0.58f,-0.77f,-0.27f}, {0.15f,0.15f,0.18f}, {0.90f,0.88f,0.85f}, {1,1,1}},
        {"Sunset",  {-0.94f,-0.31f, 0.00f}, {0.10f,0.07f,0.05f}, {1.00f,0.55f,0.25f}, {1.0f,0.8f,0.6f}},
        {"Night",   { 0.00f,-1.00f, 0.00f}, {0.03f,0.04f,0.08f}, {0.30f,0.35f,0.55f}, {0.6f,0.7f,1.0f}},
        {"Studio",  {-0.45f,-0.45f,-0.77f}, {0.08f,0.08f,0.10f}, {0.95f,0.95f,0.95f}, {1,1,1}},
        {"Dramatic",{ 0.71f,-0.41f,-0.58f}, {0.02f,0.02f,0.03f}, {1.00f,0.92f,0.85f}, {1,1,1}},
        {"Cold",    { 0.00f,-1.00f, 0.00f}, {0.05f,0.08f,0.15f}, {0.70f,0.80f,1.00f}, {1,1,1}},
    };

    int cols = 3;
    for (int i = 0; i < IM_ARRAYSIZE(presets); i++) {
        if (i % cols != 0) ImGui::SameLine();
        if (ImGui::Button(presets[i].name, {76, 0})) {
            light.direction = glm::normalize(presets[i].dir);
            light.ambient   = presets[i].amb;
            light.diffuse   = presets[i].dif;
            light.specular  = presets[i].spe;
            changed = true;
        }
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Direction");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat3("##dir", &light.direction.x, -1.f, 1.f, "%.2f"))
        changed = true;

    ImGui::Spacing();
    ImGui::SeparatorText("Colors");
    changed |= ImGui::ColorEdit3("Ambient",  &light.ambient.x,  ImGuiColorEditFlags_NoInputs);
    changed |= ImGui::ColorEdit3("Diffuse",  &light.diffuse.x,  ImGuiColorEditFlags_NoInputs);
    changed |= ImGui::ColorEdit3("Specular", &light.specular.x, ImGuiColorEditFlags_NoInputs);

    if (changed) renderer->setLight(light);
}

void UI::drawPanelCamera() {
    auto* cam = m_app->getCamera();
    if (!cam) return;

    ImGui::SeparatorText("Projection");
    float fov = cam->fov();
    if (ImGui::SliderFloat("FOV", &fov, 15.f, 120.f, "%.0f deg"))
        cam->setFov(fov);
    Tooltip("Field of view");

    ImGui::Spacing();
    ImGui::SeparatorText("Presets");
    if (ImGui::Button("Front",  {55,0})) { cam->reset(); }
    ImGui::SameLine();
    if (ImGui::Button("Top",    {55,0})) {
        cam->orbit(0, 89.f);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset",  {55,0})) {
        cam->reset();
        cam->setFov(45.f);
    }
    Tooltip("Remet la camera a sa position initiale");

    if (!m_app->getModels().empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Fit", {40,0})) {
            auto aabb = m_app->getModels()[0]->worldAABB();
            cam->fitToSphere(aabb.center(), aabb.radius() * 2.5f);
        }
        Tooltip("Cadrer le modele [F]");
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Info");
    auto pos = cam->position();
    auto tgt = cam->target();
    ImGui::Text("pos  %.2f  %.2f  %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("look %.2f  %.2f  %.2f", tgt.x, tgt.y, tgt.z);
    ImGui::Text("fov  %.0f deg", cam->fov());
}

void UI::drawStatsOverlay(const RenderStats& stats) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({io.DisplaySize.x - 160.f, 10.f}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({150, 0}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.78f);

    ImGuiWindowFlags f =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs  |
        ImGuiWindowFlags_NoNav         | ImGuiWindowFlags_NoMove   |
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("##stats", nullptr, f)) {
        ImVec4 fpsColor = m_fps >= 55 ? ImVec4{0.3f,0.9f,0.4f,1} :
                          m_fps >= 30 ? ImVec4{1.0f,0.8f,0.2f,1} :
                                        ImVec4{1.0f,0.3f,0.3f,1};
        ImGui::TextColored(fpsColor, "%.0f fps", m_fps);
        ImGui::TextDisabled("%.2f ms", stats.frameTime);
        ImGui::Separator();
        ImGui::Text("%d tris", stats.triangles);
        ImGui::TextDisabled("%d calls", stats.drawCalls);
    }
    ImGui::End();
}

void UI::drawStatusBar() {
    ImGuiIO& io = ImGui::GetIO();
    float barH = 22.f;
    ImGui::SetNextWindowPos({0, io.DisplaySize.y - barH}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({io.DisplaySize.x, barH}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.82f);

    ImGuiWindowFlags f =
        ImGuiWindowFlags_NoDecoration  | ImGuiWindowFlags_NoInputs  |
        ImGuiWindowFlags_NoNav          | ImGuiWindowFlags_NoMove    |
        ImGuiWindowFlags_NoSavedSettings| ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  {8, 3});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.06f, 0.07f, 0.09f, 0.90f});

    if (ImGui::Begin("##statusbar", nullptr, f)) {
        ImGui::TextDisabled("drag: orbit");
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("scroll: zoom");
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("F: fit");
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("Tab: next mesh");
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("W: wire");
        ImGui::SameLine(0, 16);
        ImGui::TextDisabled("1-5: shading");

        if (!m_app->getModels().empty()) {
            auto* mdl = m_app->getModels()[0].get();
            std::string info = std::string("  |  ") + mdl->name() +
                               "  " + std::to_string(mdl->triCount()) + " tris";
            ImGui::SameLine(0, 20);
            ImGui::TextColored({0.5f,0.7f,1.0f,1.f}, "%s", info.c_str());
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void UI::printHelp() {
    std::cout
        << "\n+----------------------------------------------------------+\n"
        << "|  MeshForge                                               |\n"
        << "+----------------------------------------------------------+\n"
        << "|  drag        orbit                                       |\n"
        << "|  scroll      zoom                                        |\n"
        << "|  F           fit to model                               |\n"
        << "|  Tab         next mesh                                   |\n"
        << "|  W           wireframe                                   |\n"
        << "|  1-5         shading modes                               |\n"
        << "|  Space       auto-spin                                   |\n"
        << "|  Q / Esc     quit                                        |\n"
        << "+----------------------------------------------------------+\n\n";
}

void UI::printModelInfo() {}
