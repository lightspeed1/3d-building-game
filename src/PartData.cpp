#include "include/PartData.h"
#include "include/VAOData.h"
#include <iostream>

extern glm::mat4 view;
extern glm::mat4 proj;      

/*
set the variable
change the vertex buffer + element buffer (if u wanna use it)
change vertex attrib pointer
*/
//do glm things
void PartData::scale(glm::vec3 vec)
{
    scaleVector = vec;
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleVector);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)));
}
void PartData::rotate(glm::vec3 axis, float degrees)
{
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(degrees), axis);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleVector);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)));
    for(int i = 0; i < 6; i++)
    {
        glm::vec3 hi = normalMatrix * VAOs[GAMECUBE]->normalsArray[i];
    }
}