#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint id = 0;

    Shader() = default;
    void init(const std::string& vertSrc, const std::string& fragSrc);
    void use() const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setBool(const std::string& name, bool value) const;
    void setVec2(const std::string& name, const glm::vec2& v) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;
    void setVec4(const std::string& name, const glm::vec4& v) const;
    void setMat4(const std::string& name, const glm::mat4& m) const;
    void destroy();

private:
    GLuint compileShader(GLenum type, const std::string& source);
};

class Texture {
public:
    GLuint id = 0;
    int width = 0, height = 0, channels = 0;

    Texture() = default;
    bool loadFromFile(const std::string& path);
    bool loadFromMemory(const unsigned char* data, int w, int h, int ch);
    void createEmpty(int w, int h, GLenum internalFormat = GL_RGBA16F, GLenum format = GL_RGBA, GLenum type = GL_FLOAT);
    void bind(int unit = 0) const;
    void destroy();
};

class Framebuffer {
public:
    GLuint fbo = 0;
    GLuint colorTexture = 0;
    int width = 0, height = 0;

    Framebuffer() = default;
    void create(int w, int h, GLenum internalFormat = GL_RGBA16F);
    void bind() const;
    void unbind() const;
    void bindTexture(int unit = 0) const;
    void destroy();
};

class QuadRenderer {
public:
    void init();
    void renderQuad() const;
    void destroy();

private:
    GLuint vao = 0, vbo = 0;
};
