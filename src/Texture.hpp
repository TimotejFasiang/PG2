// Texture.hpp
#pragma once
#include <GL/glew.h>
#include <string>
#include <memory>

class Texture {
public:
    static std::shared_ptr<Texture> create(const std::string& path);

    ~Texture();
    void bind(GLenum textureUnit = GL_TEXTURE0) const;
    GLuint id() const { return m_id; }
    bool valid() const { return m_id != 0; }
    bool hasAlpha() const { return m_hasAlpha; }

private:
    Texture() = default; // Private constructor
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_hasAlpha = false;
};