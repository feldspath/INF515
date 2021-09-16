
#include "window_helper.hpp"
#include "opengl_helper.hpp"

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <array>
#include <initializer_list>

// ************************************ //
//          Global variables
// ************************************ //
GLuint shader_program = 0;    // Id of the shader used to draw the data (only one shader)
GLuint vao = 0;               // Set of attributes to draw the data (only one attribute)
int counter_drawing_loop = 0; // Counter to handle the animation

struct vec3 {
	float x;
	float y;
	float z;

	vec3(std::initializer_list<float> values) {
		auto it = values.begin();
		x = *(it++);
		y = *(it++);
		z = *it;
	}
};

std::array<vec3, 8> cube_primitive_vertices = {
	vec3({-0.5f, -0.5f, -0.5f}), vec3({0.5f, -0.5f, -0.5f}), vec3({0.5f, -0.5f, 0.5f}), vec3({-0.5f, -0.5f, 0.5f}),
	vec3({-0.5f, 0.5f, -0.5f}), vec3({0.5f, 0.5f, -0.5f}), vec3({0.5f, 0.5f, 0.5f}), vec3({-0.5f, 0.5f, 0.5f})
};

std::array<int, 12 * 3> cube_primitive_indices = {
	0, 2, 1,
	0, 3, 2,
	1, 2, 5,
	2, 6, 5,
	3, 7, 2,
	2, 7, 6,
	4, 5, 6,
	4, 6, 7,
	0, 4, 3,
	3, 4, 7,
	0, 1, 4,
	1, 5, 4
};



// ************************************ //
//          Function headers
// ************************************ //
void load_data();             // Load and send data to the GPU once
void draw_data();             // Drawing calls within the animation loop


/** Main function, call the general functions and setup the animation loop */
int main()
{
	std::cout << "*** Init GLFW ***" << std::endl;
	glfw_init();

	std::cout << "*** Create window ***" << std::endl;
	auto window = glfw_create_window(500, 500, "My Window");
	glfwMakeContextCurrent(window);

	std::cout << "*** Init GLAD ***" << std::endl;
	glad_init();


	print_opengl_information();

	std::cout << "*** Setup Data ***" << std::endl;
	load_data();

	std::cout << "*** Compile Shader ***" << std::endl;
	shader_program = create_shader_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

	std::cout << "*** Start GLFW loop ***" << std::endl;
	while (!glfwWindowShouldClose(window)) {

		draw_data();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	std::cout << "*** Terminate GLFW loop ***" << std::endl;

}



/** Create (or load) data and send them to GPU */
void load_data()
{
	// *** Send data on the GPU
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, cube_primitive_vertices.size() * sizeof(GLfloat) * 3, cube_primitive_vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// *** Set shader attributes
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLuint ebo = 0;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_primitive_indices.size() * sizeof(int), cube_primitive_indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}


/** Function called within the animation loop.
	Setup uniform variables and drawing calls  */
void draw_data()
{
	// ******************************** //
	// Clear screen
	// ******************************** //
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	// ******************************** //
	// Draw data
	// ******************************** //
	glUseProgram(shader_program);
	glBindVertexArray(vao);

	// Draw call
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

	//glBindVertexArray(0);
	//glUseProgram(0);
}


