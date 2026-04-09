#include <GL/glew.h>
#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(Shader&& o) noexcept : m_id(o.m_id) { o.m_id = 0; }
Shader& Shader::operator=(Shader&& o) noexcept {
    if (this != &o) {
        if (m_id) glDeleteProgram(m_id);
        m_id = o.m_id; o.m_id = 0;
    }
    return *this;
}
Shader::~Shader() { if (m_id) glDeleteProgram(m_id); }

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[shader] Cannot open: " << path << "\n";
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool Shader::loadFromFiles(const std::string& vp, const std::string& fp) {
    auto vs = readFile(vp);
    auto fs = readFile(fp);
    if (vs.empty() || fs.empty()) return false;
    return loadFromSource(vs, fs);
}

bool Shader::loadFromFiles(const std::string& vp, const std::string& fp,
                            const std::string& gp) {
    auto vs = readFile(vp);
    auto fs = readFile(fp);
    auto gs = readFile(gp);
    if (vs.empty() || fs.empty() || gs.empty()) return false;
    auto vid = compile(GL_VERTEX_SHADER,   vs);
    auto fid = compile(GL_FRAGMENT_SHADER, fs);
    auto gid = compile(GL_GEOMETRY_SHADER, gs);
    return link(vid, fid, gid);
}

bool Shader::loadFromSource(const std::string& vs, const std::string& fs) {
    auto vid = compile(GL_VERTEX_SHADER,   vs);
    auto fid = compile(GL_FRAGMENT_SHADER, fs);
    return link(vid, fid);
}

unsigned int Shader::compile(unsigned int type, const std::string& src) {
    unsigned int id = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(id, 1, &c, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(id, 1024, nullptr, log);
        std::cerr << "[shader] Compile error ("
                  << (type == GL_VERTEX_SHADER ? "vert" :
                      type == GL_FRAGMENT_SHADER ? "frag" : "geom")
                  << "):\n" << log << "\n";
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Shader::link(unsigned int vid, unsigned int fid, unsigned int gid) {
    if (!vid || !fid) return false;
    if (m_id) glDeleteProgram(m_id);
    m_uniformCache.clear();

    m_id = glCreateProgram();
    glAttachShader(m_id, vid);
    glAttachShader(m_id, fid);
    if (gid) glAttachShader(m_id, gid);
    glLinkProgram(m_id);

    int success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(m_id, 1024, nullptr, log);
        std::cerr << "[shader] Link error:\n" << log << "\n";
        glDeleteProgram(m_id); m_id = 0;
    }

    glDeleteShader(vid);
    glDeleteShader(fid);
    if (gid) glDeleteShader(gid);
    return success;
}

void Shader::use()   const { glUseProgram(m_id); }
void Shader::unuse() const { glUseProgram(0); }

int Shader::getUniformLocation(const std::string& name) const {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) return it->second;
    int loc = glGetUniformLocation(m_id, name.c_str());
    m_uniformCache[name] = loc;
    return loc;
}

void Shader::setBool (const std::string& n, bool v)           const { glUniform1i (getUniformLocation(n), (int)v); }
void Shader::setInt  (const std::string& n, int v)            const { glUniform1i (getUniformLocation(n), v); }
void Shader::setFloat(const std::string& n, float v)          const { glUniform1f (getUniformLocation(n), v); }
void Shader::setVec2 (const std::string& n, const glm::vec2& v) const { glUniform2fv(getUniformLocation(n), 1, glm::value_ptr(v)); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(getUniformLocation(n), 1, glm::value_ptr(v)); }
void Shader::setVec4 (const std::string& n, const glm::vec4& v) const { glUniform4fv(getUniformLocation(n), 1, glm::value_ptr(v)); }
void Shader::setMat3 (const std::string& n, const glm::mat3& m) const { glUniformMatrix3fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(getUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m)); }
