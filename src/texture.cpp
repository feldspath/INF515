#include "texture.hpp"

Texture::Texture() : id(0) {}

Texture::Texture(std::vector<GLubyte> &bytes) { this->bytes = bytes; }

void Texture::send_texture_3D(GLint internalformat, GLsizei width, GLsizei height, GLsizei depth,
                              GLenum format) {
    // Texture genration
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    // Send texture to GPU
    glTexImage3D(GL_TEXTURE_3D, 0, internalformat, width, height, depth, 0, format,
                 GL_UNSIGNED_BYTE, static_cast<const void *>(bytes.data()));

    // Mipmap parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    // Filter parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Wrapping parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Unbind texture
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Texture::bind_texture(int index) const {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_3D, id);
}

const GLubyte *Texture::data() const { return bytes.data(); }