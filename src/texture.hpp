#pragma once
#include <glad/glad.hpp>
#include <vector>

class Texture {
private:
    GLuint id;
    std::vector<GLubyte> bytes;

public:
    Texture();
    Texture(std::vector<GLubyte> &bytes);
    void send_texture_3D(GLint internalformat, GLsizei width, GLsizei height, GLsizei depth,
                         GLenum format);
    void bind_texture(int index) const;
    const GLubyte *data() const;
};