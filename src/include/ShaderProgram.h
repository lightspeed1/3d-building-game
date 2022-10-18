#pragma once
#define GLEW_STATIC 1
#include "glew.h"
class ShaderProgram
{
    public:
        const char* vertexShaderText = nullptr;
        const char* fragmentShaderText = nullptr;
        GLuint programID = 0;
        ShaderProgram();
        ShaderProgram(const char* vertexShaderPtr, const char* fragmentShaderPtr);
        void createProgram();
};