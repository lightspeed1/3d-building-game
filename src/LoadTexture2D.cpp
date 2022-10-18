#define GLEW_STATIC 1
#include <iostream>
#include "include/glew.h"
#include "include/stb_image.h"
#include "include/LoadTexture2D.h"

bool texturesActive[16]{};

unsigned int loadTexture2D(const char* filePath, int* texUnitReturn)
{
    int activeTexture = -1;
    for(int i = 0; i < 16; i++)
    {
        if(texturesActive[i] == false)
        {
            texturesActive[i] = true;
            activeTexture = GL_TEXTURE0 + i;
        }
    }
    if(activeTexture < 0)
    {
        std::cout << "Error: no more texture units available.\n";
        return -1;
    }
    *texUnitReturn = activeTexture;

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(activeTexture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    int width, height, nrChannels;
    unsigned char* imageData = stbi_load(filePath, &width, &height, &nrChannels, 0);
    int format = nrChannels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(imageData);
    
    return texture;
}