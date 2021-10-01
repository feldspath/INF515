
#include "opengl_helper.hpp"
#include "window_helper.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BLOCK_VOXELS 8

// ************************************ //
//          Global variables
// ************************************ //
GLuint shader_program = 0;    // Id of the shader used to draw the data (only one shader)
GLuint vao = 0;               // Set of attributes to draw the data (only one attribute)
int counter_drawing_loop = 0; // Counter to handle the animation

auto sphere_position = glm::vec3(0.0f, 0.0f, -5.0f);
auto sphere_radius = 0.2f;

float sdf(glm::vec3 position) { return glm::distance(position, sphere_position) - sphere_radius; }

class Block {
private:
    std::array<unsigned int, BLOCK_VOXELS * BLOCK_VOXELS * BLOCK_VOXELS / 4> m_voxels;

public:
    Block() : m_voxels() {}

    Block(glm::vec3 origin, float block_size) : m_voxels() {
        auto sample_offset = glm::vec3(1.0f) * (block_size / BLOCK_VOXELS / 2);
        auto voxel_size = block_size / BLOCK_VOXELS;
        unsigned int temp = 0;
        for (int i = 0; i < BLOCK_VOXELS; ++i) {
            for (int j = 0; j < BLOCK_VOXELS; ++j) {
                for (int k = 0; k < BLOCK_VOXELS; ++k) {
                    auto position = glm::vec3(i, j, k) * voxel_size + origin + sample_offset;
                    float distance = sdf(position);
                    distance = std::clamp(distance, -4.0f, 4.0f) + 4.0f;
                    distance *= 32.0f;
                    unsigned int discrete_dist = (unsigned char)distance;
                    temp |= discrete_dist << 8 * (k % 4);
                    if (k % 4 == 3) {
                        int_at(i, j, k) = temp;
                        temp = 0;
                    }
                }
            }
        }
    }

    unsigned int &int_at(int i, int j, int k) {
        return m_voxels[(i * BLOCK_VOXELS * BLOCK_VOXELS + j * BLOCK_VOXELS + k) / 4];
    }

    unsigned char char_at(int i, int j, int k) {
        unsigned int temp = int_at(i, j, k);
        unsigned char value = (temp >> 8 * (k % 4)) & 0xff;
        return value;
    }

    const unsigned int *data() { return m_voxels.data(); }
};

void generate_distance_texture(std::vector<Block> &blocks, glm::vec3 origin, int nb_blocks,
                               float volume_size) {
    blocks.resize(nb_blocks * nb_blocks * nb_blocks);
    auto block_size = volume_size / nb_blocks;
    for (int i = 0; i < nb_blocks; ++i) {
        for (int j = 0; j < nb_blocks; ++j) {
            for (int k = 0; k < nb_blocks; ++k) {
                auto position = glm::vec3(i, j, k) * block_size + origin;
                blocks[i * nb_blocks * nb_blocks + j * nb_blocks + k] = Block(position, block_size);
            }
        }
    }
}

std::array<glm::vec3, 8> cube_primitive_vertices = {
    glm::vec3({-0.5f, -0.5f, -0.5f}), glm::vec3({0.5f, -0.5f, -0.5f}),
    glm::vec3({0.5f, -0.5f, 0.5f}),   glm::vec3({-0.5f, -0.5f, 0.5f}),
    glm::vec3({-0.5f, 0.5f, -0.5f}),  glm::vec3({0.5f, 0.5f, -0.5f}),
    glm::vec3({0.5f, 0.5f, 0.5f}),    glm::vec3({-0.5f, 0.5f, 0.5f})};

std::array<int, 12 * 3> cube_primitive_indices = {0, 2, 1, 0, 3, 2, 1, 2, 5, 2, 6, 5,
                                                  3, 7, 2, 2, 7, 6, 4, 5, 6, 4, 6, 7,
                                                  0, 4, 3, 3, 4, 7, 0, 1, 4, 1, 5, 4};

// ************************************ //
//          Function headers
// ************************************ //
void load_data(); // Load and send data to the GPU once
void draw_data(); // Drawing calls within the animation loop

glm::vec3 block_origin = glm::vec3(-0.5f) + sphere_position;
std::vector<Block> blocks;
float volume_size = 1.0f;
int nb_blocks = 5;

/** Main function, call the general functions and setup the animation loop */
int main() {
    generate_distance_texture(blocks, block_origin, nb_blocks, volume_size);

    // for (int j = 0; j < BLOCK_VOXELS; ++j) {
    //    for (int k = 0; k < BLOCK_VOXELS; ++k) {
    //        unsigned char value = block.char_at(4, j, k);
    //        std::cout << (float)value / 32.0f - 4.0f << '\t';
    //    }
    //    std::cout << '\n';
    //}

    std::cout << "*** Init GLFW ***" << std::endl;
    glfw_init();

    std::cout << "*** Create window ***" << std::endl;
    auto window = glfw_create_window(800, 600, "My Window");
    glfwMakeContextCurrent(window);

    std::cout << "*** Init GLAD ***" << std::endl;
    glad_init();

    print_opengl_information();

    std::cout << "*** Setup Data ***" << std::endl;
    load_data();

    std::cout << "*** Compile Shader ***" << std::endl;
    shader_program =
        create_shader_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    std::cout << "*** Start GLFW loop ***" << std::endl;
    while (!glfwWindowShouldClose(window)) {
        draw_data();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    std::cout << "*** Terminate GLFW loop ***" << std::endl;
}

/** Create (or load) data and send them to GPU */
void load_data() {
    GLuint vbo = 0;
    GLuint ebo = 0;

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // *** Send data on the GPU
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, cube_primitive_vertices.size() * sizeof(glm::vec3),
                 cube_primitive_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_primitive_indices.size() * sizeof(int),
                 cube_primitive_indices.data(), GL_STATIC_DRAW);

    // *** Set shader attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);
}

/** Function called within the animation loop.
        Setup uniform variables and drawing calls  */
void draw_data() {
    // ******************************** //
    // Clear screen
    // ******************************** //
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    double time = glfwGetTime();

    // ******************************** //
    // Draw data
    // ******************************** //
    auto projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    auto projection_inverse = glm::inverse(projection);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, sphere_position);
    model = glm::rotate(model, 1.5f, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f));

    glm::vec3 camera_center = {0.0f, 0.1f * cos(time), 0.0f};
    auto view = glm::mat4(1.0f);
    view = glm::translate(view, camera_center);
    // view = glm::rotate(view, time, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 light_pos = {10.0f * sin(time), 0.0f, 10.0f * cos(time)};

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE,
                       &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);

    glUniform1f(glGetUniformLocation(shader_program, "max_depth"), 100.0f);
    glUniform1i(glGetUniformLocation(shader_program, "width"), 800);
    glUniform1i(glGetUniformLocation(shader_program, "height"), 600);
    glUniform3fv(glGetUniformLocation(shader_program, "camera_center"), 1, &camera_center[0]);
    glUniform3fv(glGetUniformLocation(shader_program, "light_pos"), 1, &light_pos[0]);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection_inverse"), 1, GL_FALSE,
                       &projection_inverse[0][0]);

    glUniform1uiv(glGetUniformLocation(shader_program, "sdf_samples"), 128,
                  (unsigned int *)blocks.data());
    glUniform3fv(glGetUniformLocation(shader_program, "volume_origin"), 1, &block_origin[0]);
    glUniform1f(glGetUniformLocation(shader_program, "volume_size"), volume_size);
    glUniform1f(glGetUniformLocation(shader_program, "nb_blocks"), nb_blocks);

    // Draw call
    glDrawElements(GL_TRIANGLES, cube_primitive_indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
