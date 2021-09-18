#include <glad/glad.hpp>
#include <GLFW/glfw3.h>

#include <string>


/** Initialize GLFW (must be called before creating a window) */
void glfw_init();

/** Create a window using GLFW */
GLFWwindow* glfw_create_window(int width,int height, const std::string& title);

/** Load OpenGL functions using Glad library */
void glad_init();


	