cmake_minimum_required(VERSION 3.16)
project(MeshForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# ── GLFW ──────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# ── GLEW ──────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG        origin/master
    GIT_SHALLOW    TRUE
)
set(glew-cmake_BUILD_SHARED  OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC  ON  CACHE BOOL "" FORCE)
set(ONLY_LIBS                ON  CACHE BOOL "" FORCE)

# ── GLM ───────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# ── Dear ImGui ────────────────────────────────────────────────────────────────
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.6
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glew glm imgui)

# ImGui n'a pas de CMakeLists — on le compile manuellement
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw OpenGL::GL)
if(WIN32)
    target_compile_definitions(imgui_lib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
endif()

# ── OpenGL ────────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Camera.cpp
    src/Transform.cpp
    src/OBJLoader.cpp
    src/MeshGenerator.cpp
    src/InputHandler.cpp
    src/UI.cpp
)

add_executable(meshforge ${SOURCES})

target_include_directories(meshforge PRIVATE
    include
    ${OPENGL_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(meshforge PRIVATE
    OpenGL::GL
    glfw
    libglew_static
    glm::glm
    imgui_lib
)

target_compile_definitions(meshforge PRIVATE
    GLEW_STATIC
    GLM_ENABLE_EXPERIMENTAL
)

if(WIN32)
    target_link_libraries(meshforge PRIVATE gdi32 user32 shell32 winmm)
endif()

add_custom_command(TARGET meshforge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/models
        $<TARGET_FILE_DIR:meshforge>/models
    COMMENT "Copie des modeles..."
)

message(STATUS "MeshForge ${PROJECT_VERSION} — GLFW + GLEW + GLM + ImGui via FetchContent")
