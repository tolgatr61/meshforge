#include "Application.h"
#include <iostream>
#include <cstring>

void printUsage(const char* prog) {
    std::cout <<
        "\nMeshForge\n"
        "Usage: " << prog << " [options] [model.obj]\n\n"
        "Options:\n"
        "  -w, --width  <px>   Window width  (default: 1280)\n"
        "  -h, --height <px>   Window height (default: 720)\n"
        "  --no-msaa           Disable MSAA\n"
        "  --help              Show this message\n\n"
        "Controls:\n"
        "  Left drag           Orbit camera\n"
        "  Middle drag / Alt   Pan camera\n"
        "  Scroll              Zoom\n"
        "  F                   Fit model to view\n"
        "  W                   Toggle wireframe\n"
        "  N                   Toggle normals\n"
        "  1-7                 Shading modes\n"
        "  G                   Toggle grid\n"
        "  R                   Reset transform\n"
        "  Tab                 Cycle builtin meshes\n"
        "  Space               Auto-rotate\n"
        "  Ctrl+S              Export OBJ\n"
        "  Esc / Q             Quit\n\n";
}

int main(int argc, char** argv) {
    AppConfig config;
    std::string modelPath;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if ((std::strcmp(argv[i], "-w") == 0 ||
                    std::strcmp(argv[i], "--width") == 0) && i + 1 < argc) {
            config.width = std::stoi(argv[++i]);
        } else if ((std::strcmp(argv[i], "-h") == 0 ||
                    std::strcmp(argv[i], "--height") == 0) && i + 1 < argc) {
            config.height = std::stoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--no-msaa") == 0) {
            config.msaa = false;
        } else {

            modelPath = argv[i];
        }
    }

    Application app(config);

    if (!app.init()) {
        std::cerr << "[meshforge] Failed to initialize. Exiting.\n";
        return 1;
    }

    if (!modelPath.empty()) {
        if (!app.loadModel(modelPath)) {
            std::cerr << "[meshforge] Could not load: " << modelPath
                      << " — loading default mesh\n";
            app.loadBuiltinMesh("suzanne");
        }
    } else {

        app.loadBuiltinMesh("trefoil");
    }

    app.run();
    app.shutdown();

    return 0;
}
