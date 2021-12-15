#pragma once

#include "texture.hpp"
#include <glad/glad.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class Block {
private:
    Texture sdf_texture;
    Texture normals_texture;
    float block_size;
    glm::vec3 origin;
    int nb_texels;
    float (*sdf)(glm::vec3);

public:
    Block();
    Block(glm::vec3 origin, float block_size, int nb_texels, float (*sdf)(glm::vec3));

    void generate_textures();
    void bind_textures() const;

    void print_slice(int z) const;
};