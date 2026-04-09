#pragma once

#include "Mesh.h"

namespace MeshGenerator {

    Mesh cube(int subdivisions = 0);
    Mesh sphere(int rings = 32, int sectors = 32);
    Mesh cylinder(int segments = 32, float height = 2.0f, float radius = 1.0f,
                  bool caps = true);
    Mesh cone(int segments = 32, float height = 2.0f, float radius = 1.0f);
    Mesh torus(int rings = 32, int sides = 16,
               float outerR = 1.0f, float innerR = 0.35f);
    Mesh plane(int resX = 1, int resZ = 1, float size = 2.0f);
    Mesh icosphere(int subdivisions = 2);

    Mesh mobius(int uSteps = 64, int vSteps = 32);
    Mesh kleinBottle(int uSteps = 64, int vSteps = 32);
    Mesh trefoilKnot(int uSteps = 256, int vSteps = 16);
    Mesh superellipsoid(float e1 = 0.5f, float e2 = 0.5f, int res = 32);

    Mesh terrain(int resX = 64, int resZ = 64, float scale = 10.0f,
                 float heightScale = 1.5f, int seed = 42);

    namespace detail {
        void subdivide(std::vector<Vertex>& verts,
                       std::vector<unsigned int>& idx);
        glm::vec3 spherize(const glm::vec3& v);
    }
}
