#include "MeshGenerator.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <random>
#include <unordered_map>

using glm::vec2, glm::vec3;
static constexpr float Pi = glm::pi<float>();
static constexpr float Tau = glm::two_pi<float>();

static Vertex makeVert(vec3 p, vec3 n, vec2 uv = {}) {
    Vertex v;
    v.position  = p;
    v.normal    = glm::normalize(n);
    v.texCoords = uv;
    return v;
}

Mesh MeshGenerator::cube(int ) {
    static const vec3 normals[6] = {
        { 0, 0, 1}, { 0, 0,-1},
        {-1, 0, 0}, { 1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0}
    };
    static const vec3 positions[6][4] = {
        {{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1},{-1,-1, 1}},
        {{-1,-1,-1},{-1, 1,-1},{ 1, 1,-1},{ 1,-1,-1}},
        {{-1,-1, 1},{-1, 1, 1},{-1, 1,-1},{-1,-1,-1}},
        {{ 1,-1,-1},{ 1, 1,-1},{ 1, 1, 1},{ 1,-1, 1}},
        {{-1, 1, 1},{ 1, 1, 1},{ 1, 1,-1},{-1, 1,-1}},
        {{-1,-1,-1},{ 1,-1,-1},{ 1,-1, 1},{-1,-1, 1}}
    };
    static const vec2 uvs[4] = {{0,0},{1,0},{1,1},{0,1}};

    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    verts.reserve(24);
    idx.reserve(36);

    for (int f = 0; f < 6; f++) {
        unsigned int base = (unsigned int)verts.size();
        for (int i = 0; i < 4; i++)
            verts.push_back(makeVert(positions[f][i] * 0.5f, normals[f], uvs[i]));
        idx.insert(idx.end(), {base,base+1,base+2, base,base+2,base+3});
    }
    return Mesh(std::move(verts), std::move(idx));
}

Mesh MeshGenerator::sphere(int rings, int sectors) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;

    for (int r = 0; r <= rings; r++) {
        float phi = Pi * r / rings - Pi / 2.0f;
        float cp = std::cos(phi), sp = std::sin(phi);
        float v = (float)r / rings;

        for (int s = 0; s <= sectors; s++) {
            float theta = Tau * s / sectors;
            float ct = std::cos(theta), st = std::sin(theta);
            float u = (float)s / sectors;

            vec3 n = {cp * ct, sp, cp * st};
            verts.push_back(makeVert(n * 0.5f, n, {u, v}));
        }
    }

    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            unsigned int cur  = r * (sectors + 1) + s;
            unsigned int next = cur + sectors + 1;
            idx.insert(idx.end(), {cur, next, cur+1, next, next+1, cur+1});
        }
    }
    return Mesh(std::move(verts), std::move(idx));
}

Mesh MeshGenerator::torus(int rings, int sides, float R, float r) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= rings; i++) {
        float u = Tau * i / rings;
        float cu = std::cos(u), su = std::sin(u);

        for (int j = 0; j <= sides; j++) {
            float v = Tau * j / sides;
            float cv = std::cos(v), sv = std::sin(v);

            vec3 center = {R * cu, 0.0f, R * su};
            vec3 p = center + vec3{cv * cu * r, sv * r, cv * su * r};
            vec3 n = vec3{cv * cu, sv, cv * su};
            vec2 uv = {(float)i / rings, (float)j / sides};
            verts.push_back(makeVert(p, n, uv));
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < sides; j++) {
            unsigned int a = i * (sides + 1) + j;
            unsigned int b = a + sides + 1;
            idx.insert(idx.end(), {a, b, a+1, b, b+1, a+1});
        }
    }
    return Mesh(std::move(verts), std::move(idx));
}

Mesh MeshGenerator::trefoilKnot(int uSteps, int vSteps) {
    auto knot = [](float t) -> vec3 {
        float x = std::sin(t) + 2.0f * std::sin(2.0f * t);
        float y = std::cos(t) - 2.0f * std::cos(2.0f * t);
        float z = -std::sin(3.0f * t);
        return {x, y, z};
    };

    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    float r = 0.18f;

    for (int i = 0; i <= uSteps; i++) {
        float t  = Tau * i / uSteps;
        float t1 = Tau * (i + 1) / uSteps;

        vec3 c   = knot(t);
        vec3 c1  = knot(t1);
        vec3 T   = glm::normalize(c1 - c);
        vec3 tmp = {0, 1, 0};
        if (std::abs(glm::dot(T, tmp)) > 0.99f) tmp = {1, 0, 0};
        vec3 N = glm::normalize(glm::cross(T, glm::cross(T, tmp)));
        vec3 B = glm::cross(T, N);

        for (int j = 0; j <= vSteps; j++) {
            float v = Tau * j / vSteps;
            vec3 n = N * std::cos(v) + B * std::sin(v);
            vec3 p = c + n * r;
            verts.push_back(makeVert(p, n, {(float)i/uSteps, (float)j/vSteps}));
        }
    }

    for (int i = 0; i < uSteps; i++) {
        for (int j = 0; j < vSteps; j++) {
            unsigned int a = i * (vSteps + 1) + j;
            unsigned int b = a + vSteps + 1;
            idx.insert(idx.end(), {a, b, a+1, b, b+1, a+1});
        }
    }
    return Mesh(std::move(verts), std::move(idx));
}

Mesh MeshGenerator::mobius(int uSteps, int vSteps) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= uSteps; i++) {
        float u = Tau * i / uSteps;
        for (int j = 0; j <= vSteps; j++) {
            float v = -0.5f + (float)j / vSteps;
            float x = (1 + v * 0.5f * std::cos(u / 2)) * std::cos(u);
            float y = (1 + v * 0.5f * std::cos(u / 2)) * std::sin(u);
            float z = v * 0.5f * std::sin(u / 2);
            vec3 p = {x, y, z};
            vec3 n = glm::normalize(vec3{
                std::cos(u / 2) * std::cos(u),
                std::cos(u / 2) * std::sin(u),
                std::sin(u / 2)
            });
            verts.push_back(makeVert(p * 0.4f, n,
                                     {(float)i/uSteps, (float)j/vSteps}));
        }
    }

    for (int i = 0; i < uSteps; i++) {
        for (int j = 0; j < vSteps; j++) {
            unsigned int a = i * (vSteps + 1) + j;
            unsigned int b = a + vSteps + 1;
            idx.insert(idx.end(), {a, b, a+1, b, b+1, a+1});
        }
    }
    return Mesh(std::move(verts), std::move(idx));
}

Mesh MeshGenerator::icosphere(int subdivisions) {
    float t = (1.0f + std::sqrt(5.0f)) * 0.5f;
    std::vector<Vertex> verts;
    auto addV = [&](vec3 v) {
        v = glm::normalize(v);
        Vertex vt;
        vt.position  = v * 0.5f;
        vt.normal    = v;
        vt.texCoords = {
            0.5f + std::atan2(v.z, v.x) / Tau,
            0.5f + std::asin(v.y) / Pi
        };
        verts.push_back(vt);
        return (unsigned int)(verts.size() - 1);
    };

    addV({-1, t, 0}); addV({ 1, t, 0}); addV({-1,-t, 0}); addV({ 1,-t, 0});
    addV({ 0,-1, t}); addV({ 0, 1, t}); addV({ 0,-1,-t}); addV({ 0, 1,-t});
    addV({ t, 0,-1}); addV({ t, 0, 1}); addV({-t, 0,-1}); addV({-t, 0, 1});

    std::vector<unsigned int> idx = {
        0,11,5,  0,5,1,  0,1,7,  0,7,10, 0,10,11,
        1,5,9,   5,11,4, 11,10,2, 10,7,6, 7,1,8,
        3,9,4,   3,4,2,  3,2,6,  3,6,8,  3,8,9,
        4,9,5,   2,4,11, 6,2,10, 8,6,7,  9,8,1
    };

    for (int s = 0; s < subdivisions; s++) {
        detail::subdivide(verts, idx);
    }

    return Mesh(std::move(verts), std::move(idx));
}

void MeshGenerator::detail::subdivide(std::vector<Vertex>& verts,
                                       std::vector<unsigned int>& idx) {
    std::unordered_map<uint64_t, unsigned int> midCache;
    auto midpoint = [&](unsigned int a, unsigned int b) -> unsigned int {
        uint64_t key = (a < b) ? ((uint64_t)a << 32 | b)
                                : ((uint64_t)b << 32 | a);
        auto it = midCache.find(key);
        if (it != midCache.end()) return it->second;
        vec3 m = glm::normalize(verts[a].position + verts[b].position);
        Vertex v;
        v.position  = m;
        v.normal    = m;
        v.texCoords = (verts[a].texCoords + verts[b].texCoords) * 0.5f;
        unsigned int id = (unsigned int)verts.size();
        verts.push_back(v);
        midCache[key] = id;
        return id;
    };

    std::vector<unsigned int> newIdx;
    newIdx.reserve(idx.size() * 4);
    for (size_t i = 0; i + 2 < idx.size(); i += 3) {
        unsigned int a = idx[i], b = idx[i+1], c = idx[i+2];
        unsigned int ab = midpoint(a, b);
        unsigned int bc = midpoint(b, c);
        unsigned int ca = midpoint(c, a);
        newIdx.insert(newIdx.end(), {a,ab,ca, b,bc,ab, c,ca,bc, ab,bc,ca});
    }
    idx = std::move(newIdx);
}

Mesh MeshGenerator::terrain(int resX, int resZ, float size,
                              float heightScale, int seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    auto noise = [&](float x, float z) -> float {
        float h = 0.0f, amp = 1.0f, freq = 1.0f;
        for (int o = 0; o < 5; o++) {
            h   += amp * std::sin(freq * x * 2.3f) * std::cos(freq * z * 1.7f);
            amp  *= 0.5f; freq *= 2.1f;
        }
        return h * heightScale;
    };

    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    verts.reserve((resX + 1) * (resZ + 1));

    float dx = size / resX;
    float dz = size / resZ;
    float ox = -size * 0.5f;
    float oz = -size * 0.5f;

    for (int z = 0; z <= resZ; z++) {
        for (int x = 0; x <= resX; x++) {
            float wx = ox + x * dx;
            float wz = oz + z * dz;
            float wy = noise(wx, wz);
            Vertex v;
            v.position  = {wx, wy, wz};
            v.texCoords = {(float)x / resX, (float)z / resZ};
            v.normal    = {0, 1, 0};
            verts.push_back(v);
        }
    }

    for (int z = 0; z < resZ; z++) {
        for (int x = 0; x < resX; x++) {
            unsigned int tl = z * (resX + 1) + x;
            unsigned int tr = tl + 1;
            unsigned int bl = tl + resX + 1;
            unsigned int br = bl + 1;
            idx.insert(idx.end(), {tl, bl, tr, bl, br, tr});
        }
    }

    Mesh m(std::move(verts), std::move(idx));
    m.computeNormals();
    return m;
}

Mesh MeshGenerator::plane(int resX, int resZ, float size) {
    return terrain(resX, resZ, size, 0.0f, 0);
}

Mesh MeshGenerator::cylinder(int seg, float h, float r, bool caps) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;

    for (int i = 0; i <= seg; i++) {
        float t = Tau * i / seg;
        float ct = std::cos(t), st = std::sin(t);
        float u = (float)i / seg;
        vec3 n = {ct, 0, st};

        verts.push_back(makeVert({r*ct, -h/2, r*st}, n, {u, 0.0f}));
        verts.push_back(makeVert({r*ct,  h/2, r*st}, n, {u, 1.0f}));
    }

    for (int i = 0; i < seg; i++) {
        unsigned int a = i*2, b = i*2+1, c = i*2+2, d = i*2+3;
        idx.insert(idx.end(), {a, c, b, b, c, d});
    }

    if (caps) {

        unsigned int bot = (unsigned int)verts.size();
        verts.push_back(makeVert({0,-h/2,0}, {0,-1,0}));
        for (int i = 0; i <= seg; i++) {
            float t = Tau * i / seg;
            verts.push_back(makeVert({r*std::cos(t),-h/2,r*std::sin(t)},
                                     {0,-1,0}));
        }
        for (int i = 0; i < seg; i++)
            idx.insert(idx.end(), {bot, bot+i+2, bot+i+1});

        unsigned int top = (unsigned int)verts.size();
        verts.push_back(makeVert({0,h/2,0}, {0,1,0}));
        for (int i = 0; i <= seg; i++) {
            float t = Tau * i / seg;
            verts.push_back(makeVert({r*std::cos(t),h/2,r*std::sin(t)},
                                     {0,1,0}));
        }
        for (int i = 0; i < seg; i++)
            idx.insert(idx.end(), {top, top+i+1, top+i+2});
    }

    return Mesh(std::move(verts), std::move(idx));
}
