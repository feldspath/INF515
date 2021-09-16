#pragma once

#include <glad/glad.hpp>
#include <string>


/** Print a set of information on the OpenGL version */
void print_opengl_information();

/** Compile an individual shader provided as a string.
 * Check that the compilation succeed. */
GLuint compile_shader(const std::string& shader_str, const GLenum shader_type);


/** Compile vertex and fragment shaders provided from their file paths, link them, and return a shader program.
 * Check that link operation succeed. */
GLuint create_shader_program(const std::string& vertex_shader_path, const std::string& fragment_shader_path);

