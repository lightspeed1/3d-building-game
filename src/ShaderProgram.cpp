#define GLEW_STATIC 1
#include "include/glew.h"
#include "include/ShaderProgram.h"
#include <vector>
#include <iostream>
ShaderProgram::ShaderProgram() = default;

ShaderProgram::ShaderProgram(const char* vertexShaderPtr, const char* fragmentShaderPtr)
{
    vertexShaderText = vertexShaderPtr;
    fragmentShaderText = fragmentShaderPtr;
}

//creates a shader program (compiles and links the vertex and fragment shader source files)
void ShaderProgram::createProgram()
{
    
    //compile vertex shader
    if(!vertexShaderText || !fragmentShaderText)
        return;
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, &vertexShaderText, 0);
    glCompileShader(vertexShaderObject);

    
    GLint isCompiled = 0;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &isCompiled);
    
    //compile fragment shader
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, &fragmentShaderText, 0);
    glCompileShader(fragmentShaderObject);

    GLint isCompiled1 = 0;
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &isCompiled1);

    //link vertex and fragment shader to create program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderObject);
    glAttachShader(shaderProgram, fragmentShaderObject);
    glLinkProgram(shaderProgram);
    glDeleteShader(fragmentShaderObject);
    glDeleteShader(vertexShaderObject);
    programID = shaderProgram;
    return;
}