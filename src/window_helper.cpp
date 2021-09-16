
#include "window_helper.hpp"

#include <iostream>


void glfw_init()
{
    const int glfw_init_value = glfwInit();
    if( glfw_init_value != GLFW_TRUE ) {
        std::cerr<<"Failed to Init GLFW"<<std::endl;
        abort();
    }
}


GLFWwindow* glfw_create_window(int width,int height, const std::string& title)
{
    // Indicate to GLFW to setup context compatible with OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), /*monitor*/ nullptr, /*share*/ nullptr);
    if( window==nullptr ) {
        std::cerr<<"Failed to create GLFW Window"<<std::endl;
        abort();
    }

    return window;
}

void glad_init()
{
    const int glad_init_value = gladLoadGL();
    if( glad_init_value == 0 ) {
        std::cerr<<"Failed to Init GLAD"<<std::endl;
        abort();
    }
}
