#pragma once
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"
#include "PartData.h"

glm::vec3 lineIntersectPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 planeNormal, glm::vec3 planeOrigin);
bool SAT(PartData p1, PartData p2, int p1Type, int p2Type);
void cylinderIntersect();


struct PhysicsData
{
    glm::vec3 linearV{0.0f,0.0f,0.0f};
    glm::vec3 angularV{0.0f,0.0f,0.0f};
    glm::vec3 linearA{0.0f,-9.8f,0.0f};
    glm::vec3 angularA{0.0f,0.0f,0.0f};
};