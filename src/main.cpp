
#include "window_helper.hpp"
#include "opengl_helper.hpp"

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <array>
#include <initializer_list>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
		if (values.size() != 3) {
			std::cout << "vec::invalid initializer size!\n";
		}
		auto it = values.begin();
		x = *(it++);
		y = *(it++);
		z = *it;
	}
};

std::ostream& operator<<(std::ostream& out, const vec3& vec) {
	out << vec.x << '\t' << vec.y << '\t' << vec.z;
	return out;
}

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
	auto window = glfw_create_window(800, 600, "My Window");
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
	GLuint vbo = 0;
	GLuint ebo = 0;

	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	// *** Send data on the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, cube_primitive_vertices.size() * sizeof(vec3), cube_primitive_vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_primitive_indices.size() * sizeof(int), cube_primitive_indices.data(), GL_STATIC_DRAW);

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
void draw_data()
{
	// ******************************** //
	// Clear screen
	// ******************************** //
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	float time = glfwGetTime();

	// ******************************** //
	// Draw data
	// ******************************** //
	auto projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	auto projection_inverse = glm::inverse(projection);
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
	model = glm::rotate(model, 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 camera_center = { 0.0f, 0.1f * cos(time), 0.0f };
	auto view = glm::mat4(1.0f);
	view = glm::translate(view, camera_center);
	//view = glm::rotate(view, time, glm::vec3(1.0f, 0.0f, 0.0f));
	
	glm::vec3 light_pos = { 10.0f * sin(time), 0.0f, 10.0f * cos(time) };

	glUseProgram(shader_program);
	glBindVertexArray(vao);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);

	glUniform1f(glGetUniformLocation(shader_program, "max_depth"), 100.0f);
	glUniform1i(glGetUniformLocation(shader_program, "width"), 800);
	glUniform1i(glGetUniformLocation(shader_program, "height"), 600);
	glUniform3fv(glGetUniformLocation(shader_program, "camera_center"), 1, &camera_center[0]);
	glUniform3fv(glGetUniformLocation(shader_program, "light_pos"), 1, &light_pos[0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection_inverse"), 1, GL_FALSE, &projection_inverse[0][0]);



	// Draw call
	glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}


