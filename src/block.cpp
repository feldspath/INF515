#include <algorithm>
#include <iostream>

#include "block.hpp"

Block::Block() : block_size{0.0f}, nb_texels{0}, sdf{nullptr} {}

Block::Block(glm::vec3 origin, float block_size, int nb_texels, float (*sdf)(glm::vec3)) {
    this->block_size = block_size;
    this->origin = origin;
    this->nb_texels = nb_texels;
    this->sdf = sdf;
}

static const glm::vec3 dir1 = glm::vec3(1.0f, -1.0f, -1.0f);
static const glm::vec3 dir2 = glm::vec3(-1.0f, -1.0f, 1.0f);
static const glm::vec3 dir3 = glm::vec3(-1.0f, 1.0f, -1.0f);
static const glm::vec3 dir4 = glm::vec3(1.0f, 1.0f, 1.0f);
static const float h = 0.0001;

void Block::generate_textures() {
    auto texel_size = block_size / nb_texels;
    auto sample_offset = glm::vec3(1.0f) * (texel_size / 2);

    std::vector<GLubyte> sdf_bytes(nb_texels * nb_texels * nb_texels);
    std::vector<GLubyte> normals_bytes(3 * nb_texels * nb_texels * nb_texels);
    auto it_sdf = sdf_bytes.begin();
    auto it_normals = normals_bytes.begin();

    for (int z = 0; z < nb_texels; ++z) {
        for (int y = 0; y < nb_texels; ++y) {
            for (int x = 0; x < nb_texels; ++x) {
                auto sample_position = glm::vec3(x, y, z) * texel_size + origin + sample_offset;
                // distance
                float distance = sdf(sample_position);
                distance = std::clamp(distance, -4.0f, 4.0f) + 4.0f;
                distance *= 32.0f;
                *it_sdf++ = static_cast<GLubyte>(distance);

                // normal
                glm::vec3 normal = glm::normalize(dir1 * sdf(sample_position + dir1 * h) +
                                                  dir2 * sdf(sample_position + dir2 * h) +
                                                  dir3 * sdf(sample_position + dir3 * h) +
                                                  dir4 * sdf(sample_position + dir4 * h));
                normal = (normal + glm::vec3(1.0f, 1.0f, 1.0f)) * 128.0f;
                GLubyte normal_x = static_cast<GLubyte>(normal.x);
                GLubyte normal_y = static_cast<GLubyte>(normal.y);
                GLubyte normal_z = static_cast<GLubyte>(normal.z);
                *it_normals++ = normal_x;
                *it_normals++ = normal_y;
                *it_normals++ = normal_z;
            }
        }
    }

    sdf_texture = Texture(std::move(sdf_bytes));
    sdf_texture.send_texture_3D(GL_R8, nb_texels, nb_texels, nb_texels, GL_RED);

    normals_texture = Texture(std::move(normals_bytes));
    normals_texture.send_texture_3D(GL_RGB8, nb_texels, nb_texels, nb_texels, GL_RGB);
}

void Block::bind_textures() const {
    sdf_texture.bind_texture(0);
    normals_texture.bind_texture(1);
}

void Block::print_slice(int z) const {
    for (int y = 0; y < nb_texels; ++y) {
        for (int x = 0; x < nb_texels; ++x) {
            GLubyte discrete_dist =
                sdf_texture.data()[z * nb_texels * nb_texels + y * nb_texels + x];
            float distance = static_cast<float>(discrete_dist) / 32.0f - 4.0f;
            std::cout << distance << '\t';
        }
        std::cout << '\n';
    }
}