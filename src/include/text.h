#pragma once
#include <string>
#include <unordered_map>
struct textInfo
{
    float totalWidth;
    float maxTop;
    float minBottom;
};
struct Character {
    unsigned int textureID;  //ID of char texture
    glm::ivec2   size;       //size of char
    glm::ivec2   bearing;    //offset from origin to top left of char
    unsigned int advance;    //offset to advance to next char
};
extern std::unordered_map<char, Character> characters;
extern textInfo getTextInfo(std::string text, float scale);
extern glm::mat4 orthoProj;
extern int setupFont();
extern void drawText(std::string text, glm::vec3 color, float x, float y, float scale);
