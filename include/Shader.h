#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath,
                       const std::string& geomPath);
    bool loadFromSource(const std::string& vertSrc, const std::string& fragSrc);

    void use() const;
    void unuse() const;

    void setBool (const std::string& name, bool value) const;
    void setInt  (const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2 (const std::string& name, const glm::vec2& v) const;
    void setVec3 (const std::string& name, const glm::vec3& v) const;
    void setVec4 (const std::string& name, const glm::vec4& v) const;
    void setMat3 (const std::string& name, const glm::mat3& m) const;
    void setMat4 (const std::string& name, const glm::mat4& m) const;

    unsigned int id() const { return m_id; }
    bool isValid()    const { return m_id != 0; }

private:
    unsigned int compile(unsigned int type, const std::string& src);
    bool         link(unsigned int vert, unsigned int frag,
                      unsigned int geom = 0);
    int          getUniformLocation(const std::string& name) const;

    unsigned int m_id = 0;
    mutable std::unordered_map<std::string, int> m_uniformCache;
};
