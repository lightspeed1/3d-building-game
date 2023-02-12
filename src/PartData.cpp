#include "include/PartData.h"
#include "include/VAOData.h"
#include <iostream>

extern glm::mat4 view;
extern glm::mat4 proj;

//scale the object up or down
void PartData::scale(glm::vec3 vec)
{
    scaleVector = vec;
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleVector);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)));
}

//rotate whole object
void PartData::rotate(glm::vec3 axis, float rads)
{
    rotationMatrix = glm::rotate(glm::mat4(1.0f), rads, glm::normalize(axis)) * rotationMatrix;

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleVector);
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)));
    // for(int i = 0; i < 6; i++)
    // {
    //     glm::vec3 hi = normalMatrix * VAOs[GAMECUBE]->normalsArray[i];
    // }
}

//properly rotate normal vector
glm::vec3 PartData::applyNM(glm::vec3 normal)
{
    return(glm::normalize(normalMatrix * normal));
}


//convert a point on the part from local to world coordinates
glm::vec3 PartData::localToWorld(glm::vec3 pt)
{
    return ((glm::mat3(rotationMatrix) * (scaleVector * pt)) + translate);
}

//deep copy one PartData object into another
PartData PartData::operator=(PartData& part)
{
    for(int i = 0; i < 4; i++)
        rotationMatrix[i] = part.rotationMatrix[i];
    translate = part.translate;
    color = part.color;
    scaleVector = part.scaleVector;
    for(int i = 0; i < 3; i++)
        normalMatrix[i] = part.normalMatrix[i];
    texUnit = part.texUnit;
    return *this;
}