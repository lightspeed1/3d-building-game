#pragma once
#include <vector>

struct texture
{
    std::string texturePath = "";
    int textureUnit = -1;
    unsigned int textureID;
};

extern std::vector<bool> texturesActive;
extern int findOpenTexUnit();
extern unsigned int loadTexture2D(const char* filePath, int* texUnitReturn);