
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

// ************************************ //
//          Global variables
// ************************************ //
GLuint shader_program = 0;    // Id of the shader used to draw the data (only one shader)
GLuint vao = 0;               // Set of attributes to draw the data (only one attribute)
int counter_drawing_loop = 0; // Counter to handle the animation

auto sphere_position = glm::vec3(0.0f, 0.0f, -2.0f);
auto sphere_radius = 0.2f;

float sdf(glm::vec3 position) { return glm::distance(position, sphere_position) - sphere_radius; }

class Block {
private:
    std::vector<GLubyte> texture_bytes;
    GLuint texture_id;
    float block_size;
    glm::vec3 origin;
    int nb_texels;

public:
    Block() : texture_id{0}, block_size{0.0f} {}

    Block(glm::vec3 origin, float block_size, int nb_texels) : texture_id(-1) {
        this->block_size = block_size;
        this->origin = origin;
        this->nb_texels = nb_texels;
    }

    const GLubyte *data() const { return texture_bytes.data(); }

    void generate_texture() {
        auto texel_size = block_size / nb_texels;
        auto sample_offset = glm::vec3(1.0f) * (texel_size / 2);

        texture_bytes.resize(nb_texels * nb_texels * nb_texels);
        auto it = texture_bytes.begin();

        for (int z = 0; z < nb_texels; ++z) {
            for (int y = 0; y < nb_texels; ++y) {
                for (int x = 0; x < nb_texels; ++x) {
                    auto position = glm::vec3(x, y, z) * texel_size + origin + sample_offset;
                    float distance = sdf(position);
                    distance = std::clamp(distance, -4.0f, 4.0f) + 4.0f;
                    distance *= 32.0f;
                    *it++ = static_cast<GLubyte>(distance);
                }
            }
        }
    }

    void send_texture() {

        // Texture genration
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_3D, texture_id);

        // Send texture to GPU
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, nb_texels, nb_texels, nb_texels, 0, GL_RED,
                     GL_UNSIGNED_BYTE, static_cast<const void *>(data()));

        // Mipmap parameters
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

        // Filter parameters
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Wrapping parameters
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        float borderColor[] = {0.5f / 8.0f + 0.5f, 0.0f, 0.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Unbind texture
        glBindTexture(GL_TEXTURE_3D, 0);
    }

    void bind_texture() { glBindTexture(GL_TEXTURE_3D, texture_id); }

    void print_slice(int z) {
        for (int y = 0; y < nb_texels; ++y) {
            for (int x = 0; x < nb_texels; ++x) {
                GLubyte discrete_dist =
                    texture_bytes[z * nb_texels * nb_texels + y * nb_texels + x];
                float distance = static_cast<float>(discrete_dist) / 32.0f - 4.0f;
                std::cout << distance << '\t';
            }
            std::cout << '\n';
        }
    }
};

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
Block block;
float volume_size = 1.0f;
int nb_texels = 16;

/** Main function, call the general functions and setup the animation loop */
int main() {
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

    block = Block(block_origin, volume_size, nb_texels);
    block.generate_texture();
    block.send_texture();
    block.print_slice(8);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

/** Function called within the animation loop.
        Setup uniform variables and drawing calls  */
void draw_data() {
    // ******************************** //
    // Clear screen
    // ******************************** //
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    double time = glfwGetTime();

    // ******************************** //
    // Draw data
    // ******************************** //
    auto projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    auto projection_inverse = glm::inverse(projection);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, sphere_position);
    // model = glm::rotate(model, 1.5f, glm::vec3(1.0f, 0.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(1.5f));

    // glm::vec3 camera_center = {0.0f, 0.1f * cos(time), 0.0f};
    glm::vec3 camera_center = {0.0f, 0.0f, 0.0f};
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

    // Pass texture to shader
    block.bind_texture();

    glUniform3fv(glGetUniformLocation(shader_program, "volume_origin"), 1, &block_origin[0]);
    glUniform1f(glGetUniformLocation(shader_program, "volume_size"), volume_size);
    glUniform1f(glGetUniformLocation(shader_program, "nb_texels"), nb_texels);

    // Draw call
    glDrawElements(GL_TRIANGLES, cube_primitive_indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}
