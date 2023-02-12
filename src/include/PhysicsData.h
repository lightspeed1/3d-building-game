#pragma once
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"
#include "PartData.h"
#include <vector>
#include <utility>

#define NORMAL 0
#define EXPLOSION 1

extern glm::vec3 lineIntersectPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 planeNormal, glm::vec3 planeOrigin);
extern int lineSegmentsIntersect(glm::vec3 p1Line1, glm::vec3 p2Line1, glm::vec3 p1Line2, glm::vec3 p2Line2, glm::vec3* intPts);
extern bool SAT(int p1Index, int p2Index, glm::vec3* intersectionData);
extern void cylinderIntersect();
extern void physicsTick();
extern bool fequal(float a, float b);
extern bool fGEqual(float a, float b);
extern bool v3Equal(glm::vec3 one, glm::vec3 two);
extern void sortPointsArr(std::vector<glm::vec3>& arr, std::vector<std::pair<int,int>>& result);
extern bool inShape(std::vector<glm::vec3>& shapePoints, std::vector<std::pair<int,int>> shapeEdges, glm::vec3 point);
extern bool pointInPart(int partIndex, glm::vec3 pt);
extern void setVelsZero();
extern void createExplosion(glm::vec3 pos, float radius, float strength);
struct PhysicsData
{
    glm::vec3 linearV{0.0f,0.0f,0.0f};
    glm::vec3 angularV{0.0f,0.0f,0.0f};
    glm::vec3 linearA{0.0f,0.0f,0.0f};
    glm::vec3 angularA{0.0f,0.0f,0.0f};
    float mass{1.0f};
    int newIndex;
    bool stationary = false;
    // int partAttribs = NORMAL;
};