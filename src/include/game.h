#pragma once
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"
#include "include/DrawPrims.h"
#include <chrono>
#define FREEZE 1
#define UNFREEZE 0
#define PLAYING 2
#define SAVEMENU 3
#define TITLESCREEN 4
#define INSTRUCTIONS 5
extern float deltaTime;
extern int gameMode;
extern int gameState;
extern bool startTiming;
extern glm::mat4 view;
extern glm::mat4 proj;
extern glm::vec3 camPos;
extern glm::vec3 lightPos;
extern float pixelWidth;
extern float pixelHeight;
extern glm::vec3 yawPitchToVec(float theYaw, float thePitch);
extern bool rayIntersectsPart(int partIndex, bool returnUnselected, glm::vec3 rayOrigin, glm::vec3 ray, float* lenToIntPt, glm::vec3* intPt, int* intNormIndex);
extern int partsRayIntersects(bool noScaleParts, glm::vec3 ray, glm::vec3 rayOrigin, glm::vec3* intPtClosest, int* intNormIndex1, bool returnUnselected);
extern void printVec3(glm::vec3 v);
extern void printMat3(glm::mat3 mat);
extern std::vector<glm::vec3> normalsFromRays(glm::vec3* rays);
extern glm::vec3 screenPosToRay(int mouseX, int mouseY);
extern void rotateSelected(int key);
extern void removeFromVec(std::vector<int>& vec, int a);
extern void removeFromAdjacent(int ind);
extern void detachPart(int p);
extern bool draggedCheckCollide(int partIndex, int farIndex, std::vector<int>& touching, bool requireTouchingFar);

extern void dragParts();
extern glm::vec3 shootRay();
extern void killGroup(group* groupPtr, bool setMemsToNull);
extern glm::vec3 maxPtInDir(int partIndex, glm::vec3 dir);
extern glm::vec2 screenCoordsToNDC(float x, float y);
extern glm::vec2 NDCToScreenCoords(float x, float y);
extern buttonData* currTextBox;
extern bool colorTB;
float newPrecision(float x, int precision);
std::string vec3ToString(glm::vec3 v);
extern std::chrono::steady_clock::time_point start1;
extern std::chrono::steady_clock::time_point end1;
extern int scrollAmount;
extern bool draggingScalePart;
//If true, then object must be scaled when scale parts are dragged, otherwise, parts will be moved
extern bool scaleSelected;
extern void toggleScaleOrMove();
extern int scalePartInd;
extern glm::vec3 intPtClose;