#define GLEW_STATIC 1
#include <iostream>
#include "include/glew.h"
#include "include/stb_image.h"
#include "include/LoadTexture2D.h"

std::vector<bool> texturesActive(16, false);

//make sure there is an open texture unit, then return it.
int findOpenTexUnit()
{
    int activeTexture = -1;
    for(int i = 0; i < texturesActive.size(); i++)
    {
        if(texturesActive[i] == false)
        {
            texturesActive[i] = true;
            activeTexture = (int)(GL_TEXTURE0 + i);
            break;
        }
    }
    if(activeTexture == -1)
        std::cout << "Error: no more texture units available.\n";
    return activeTexture;
}

//load a texture from file
unsigned int loadTexture2D(const char* filePath, int* texUnitReturn)
{
    int activeTexture = findOpenTexUnit();
    if(activeTexture == -1)
        return -1;
    *texUnitReturn = activeTexture;

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(activeTexture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    int width, height, nrChannels;
    unsigned char* imageData = stbi_load(filePath, &width, &height, &nrChannels, 0);
    int format = nrChannels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(imageData);
    
    return texture;
}