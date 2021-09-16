#include "opengl_helper.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>


// Read a file given by its path and return its content as a string
static std::string read_file_text(const std::string& path)
{
    // Open file with pointer at last position
    std::ifstream stream(path, std::ios::ate);

    // Abort program if the file cannot be opened
    if( !stream.is_open() ) {
        std::cerr<<"\n\nError: cannot open file ["<<path<<"]"<<std::endl;
        std::cerr<<"=> If the file exists, you may need to adapt the path from which you run the program"<<std::endl;
        abort();
    }

    std::string full_text;

    // get position of the current pointer (last position)
    std::streampos const s = stream.tellg();

    // resize the buffer to handle the entire text
    full_text.resize( static_cast<size_t>(s) );

    // goes back to the beginning of the file and read the entire text
    stream.seekg(0,std::ios::beg);
    stream.read(&full_text[0],s);

    stream.close();
    assert(!stream.is_open());


    return full_text;

}


void print_opengl_information()
{
    std::cout<<"OpenGL information: VENDOR      : "<<glGetString(GL_VENDOR)<<std::endl;
    std::cout<<"                    RENDERDER   : "<<glGetString(GL_RENDERER)<<std::endl;
    std::cout<<"                    VERSION     : "<<glGetString(GL_VERSION)<<std::endl;
    std::cout<<"                    GLSL VERSION: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
}

GLuint compile_shader(const std::string& shader_str, const GLenum shader_type)
{
    const GLuint shader = glCreateShader(shader_type); assert( glIsShader(shader) );
    char const* const shader_cstring = shader_str.c_str();
    glShaderSource(shader, 1, &shader_cstring, nullptr);

    // Compile shader
    glCompileShader( shader );

    // Check compilation
    {
        GLint is_compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);

        // get info on compilation
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog( static_cast<size_t>(maxLength+1) );
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        if( maxLength >= 1 )
        {
            std::cerr << "[Info from shader compilation]"<< std::endl;
            std::cerr << &infoLog[0] << std::endl;
            std::cerr << "For shader " << shader_str << std::endl;
        }

        if( is_compiled==GL_FALSE )
        {
            glDeleteShader(shader);
            abort();
        }
    }

    return shader;
}

GLuint create_shader_program(const std::string& vertex_shader_path, const std::string& fragment_shader_path)
{

    const std::string vertex_shader_str   = read_file_text(vertex_shader_path);
    const std::string fragment_shader_str = read_file_text(fragment_shader_path);

    const GLuint vertex_shader   = compile_shader(vertex_shader_str, GL_VERTEX_SHADER);
    const GLuint fragment_shader = compile_shader(fragment_shader_str, GL_FRAGMENT_SHADER);

    assert( glIsShader(vertex_shader) );
    assert( glIsShader(fragment_shader) );

    // Create Program
    const GLuint program = glCreateProgram();
    assert( glIsProgram(program) );

    // Attach Shader to Program
    glAttachShader( program, vertex_shader );
    glAttachShader( program, fragment_shader );

    // Link Program
    glLinkProgram( program );

    // Check Link
    {
        GLint is_linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &is_linked);

        // Get info on Link
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog( static_cast<size_t>(maxLength+1) );
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        if( maxLength >= 1 )
        {
            std::cout << "[Info from shader Link]"<< std::endl;
            std::cout << &infoLog[0] << std::endl;
        }
        if( is_linked==GL_FALSE ) //if failed link
        {
            // clean memory
            glDeleteProgram( program );
            glDeleteShader( vertex_shader);
            glDeleteShader( fragment_shader);

            abort();
        }


        // Shader can be detached.
        glDetachShader( program, vertex_shader);
        glDetachShader( program, fragment_shader);

    }

    return program;

}

