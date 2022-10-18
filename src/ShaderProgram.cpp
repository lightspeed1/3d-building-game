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

void ShaderProgram::createProgram()
{
    
    if(!vertexShaderText || !fragmentShaderText)
        return;
    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, &vertexShaderText, 0);
    glCompileShader(vertexShaderObject);

    
    GLint isCompiled = 0;
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(vertexShaderObject, maxLength, &maxLength, &errorLog[0]);

        std::cout << "ERROR: ";
        for(int i =0; i < errorLog.size(); i++)
        {
            std::cout << errorLog[i];
        }
        std::cout << '\n';
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(vertexShaderObject); // Don't leak the shader.
        return;
    }
    
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, &fragmentShaderText, 0);
    glCompileShader(fragmentShaderObject);

    GLint isCompiled1 = 0;
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &isCompiled1);
    std::cout << isCompiled1 << ' ' << isCompiled << '\n';
    if(isCompiled1 == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(fragmentShaderObject, maxLength, &maxLength, &errorLog[0]);

        std::cout << "ERROR: ";
        for(int i =0; i < errorLog.size(); i++)
        {
            std::cout << errorLog[i];
        }
        std::cout << '\n';
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(fragmentShaderObject); // Don't leak the shader.
        return;
    }



    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderObject);
    glAttachShader(shaderProgram, fragmentShaderObject);
    glLinkProgram(shaderProgram);
    glDeleteShader(fragmentShaderObject);
    glDeleteShader(vertexShaderObject);
    programID = shaderProgram;
    return;
}