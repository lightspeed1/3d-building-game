#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define GLEW_STATIC 1
#include "include/glew.h"
#include "include/ShaderProgram.h"
#include "include/LoadTexture2D.h"
#include "../glfw-3.3.7.bin.WIN64/include/GLFW/glfw3.h"
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#include "../glm/glm/gtx/intersect.hpp"
#include "include/VAOData.h"
#include "include/ShaderSources.h"
#include "include/PhysicsData.h"
#include "include/DrawPrims.h"
#include "include/building.h"
#include "include/game.h"
#include "include/saveload.h"
#include <algorithm>
#include <sstream>
#include <text.h>
#include <chrono>

// using namespace std::chrono;
std::chrono::steady_clock::time_point start1;
std::chrono::steady_clock::time_point end1;
bool startTiming;
using namespace glm;
using namespace std;
int gameMode = FREEZE;
int gameState = TITLESCREEN;
float pixelWidth = 640.0f;
float pixelHeight = 480.0f;
float aspectRatio = pixelWidth/pixelHeight;

bool renderSaveMenu = false;
bool renderTitleScreen = true;

//converts window coordinates to normalized device coordinates (what opengl uses)
glm::vec2 screenCoordsToNDC(float x, float y)
{
    glm::vec2 dims = getViewportDims();
    dims.x = pixelWidth;
    dims.y = pixelHeight;
    float xNDC = (x - dims.x/2.0f)/(dims.x/2.0f);
    float yNDC = (y - dims.y/2.0f)/(dims.y/2.0f);
    return (glm::vec2(xNDC, yNDC));
}

glm::vec2 NDCToScreenCoords(float x, float y)
{
    glm::vec2 dims = getViewportDims();
    dims.x = pixelWidth;
    dims.y = pixelHeight;
    float xScreen = (x + 1.0f) * dims.x/2.0f;
    float yScreen = (y + 1.0f) * dims.y/2.0f;
    return(glm::vec2(xScreen, yScreen));
}

//field of view
float fovY = 45.0f;
float fovX = glm::degrees(2 * atanf(tanf(glm::radians(fovY/2.0f)) * aspectRatio));
float depth = (pixelWidth/2.0f)/tan(glm::radians(fovX/2.0f));

glm::mat4 view;
glm::mat4 proj = glm::perspective(glm::radians(fovY), aspectRatio, 0.1f, 100.0f);

float frame1Time, frame2Time, deltaTime;

//camera information
glm::vec3 camPos{0.0f, 0.0f, 6.0f};
glm::vec3 camFront{0.0f, 0.0f, -1.0f};
glm::vec3 camUp{0.0f, 1.0f, 0.0f};
glm::vec3 camRight;
glm::vec3 camUpReal = glm::cross(camRight, camFront);
glm::vec3 lightPos{4.0f,10.0f,5.0f};

short movement[4];
float speed;
float lastX, lastY;
float currYaw = 270.0f, currPitch = 0;

//right mouse button pressed
bool rightPressed = false;
bool rightPressedLastMove = false;

int mouseX;
int mouseY;

GLFWwindow* win;


//global variables for selecting multiple parts
bool ctrlHeld = false;
glm::vec2 startSelection, endSelection;
bool multipleSelect = false;
bool leftClickHeld = false;
glm::vec2 multSelectPositions[4];
glm::vec3 multSelectVerts[4];
GLFWcursor* cursor;

void printVec3(glm::vec3 v)
{
    std::cout << v[0] << ", " << v[1] << ", " << v[2] << '\n';
    return;
}

void printMat3(glm::mat3 mat)
{
    for(int i = 0; i < 3; i++)
    {
        std::cout << mat[0][i] << ", " << mat[1][i] << ", " << mat[2][i] << '\n';
    }
}
void error_callback(int error, const char* description)
{
    std::cout << "ERROR " << error << '\n' << description << '\n';
}

//converts a yaw and pitch value (in degrees) to a normalized vector pointing in that direction
glm::vec3 yawPitchToVec(float theYaw, float thePitch)
{
    glm::vec3 result;
    result.x = cos(glm::radians(theYaw)) * cos(glm::radians(thePitch));
    result.z = sin(glm::radians(theYaw)) * cos(glm::radians(thePitch));
    result.y = sin(glm::radians(thePitch));
    result = glm::normalize(result);
    return result;
}



bool mousePosSortVecHelper(std::vector<float> one, std::vector<float> two)
{
    return(one[2] < two[2]);
}

//checks if we have moved the mouse. if so, we may need to move a part we are dragging, or rotate the camera.
void mousePosCallback(GLFWwindow* window, double x, double y)
{
    
    if(gameMode == FREEZE)
    {
        if(draggingScalePart)
            scalePartDrag(selectedParts, scalePartInd, intPtClose, scaleSelected);
        else
            dragParts();
    }
    //only rotate camera if right mouse button is held
    if(!rightPressed)
    {
        rightPressedLastMove = false;
        return;
    }
    if(rightPressed && !rightPressedLastMove)
    {
        rightPressedLastMove = true;
        lastX = x;
        lastY = y;
    }
    static bool called = false;
    //this check is needed because x and y of mouse will be some unknown position when program starts.
    if(!called)
    {
        lastX = x;
        lastY = y;
        called = true;
    }
    else
    {
        //if right button pressed, we can change the rotation of the camera (changing camera information vectors)
        float changeX = x - lastX, changeY = y - lastY;
        currYaw += changeX * 0.25;
        currPitch -= changeY * 0.25;
        if(currPitch <= -90.0f)
            currPitch = -89.5f;
        if(currPitch >= 90.0f)
            currPitch = 89.5f;
        if(currYaw >= 360.0f)
            currYaw = 0.0f;
        if(currYaw < 0.0f)
            currYaw = 360.0f;
        camFront = yawPitchToVec(currYaw, currPitch);
        camRight = glm::normalize(glm::cross(camFront, camUp));
        camUpReal = glm::normalize(glm::cross(camRight, camFront));
        // std::cout << "yaw = " << yaw << " pitch = " << pitch << '\n';
        lastX = x;
        lastY = y;
    }
}

//converts a position on the screen into a vector whose origin is the camera.
glm::vec3 screenPosToRay(int x, int y)
{
    //x and y from center in pixels
    float xFromCenter = (x - (pixelWidth/2.0f));
    float yFromCenter = (y - (pixelHeight/2.0f));

    //we need to calculate an arbitrary depth of triangle to get the yaw and pitch angles
    float addYaw = atan(xFromCenter/depth);
    float addPitch = atan(yFromCenter/depth);
    glm::mat4 rotMat = glm::rotate(glm::rotate(glm::mat4(1.0f), addPitch, -camRight), addYaw, -camUpReal);
    glm::vec3 ray = rotMat * glm::vec4(camFront, 1.0f);
    return ray;
}
//get mouse position and find ray/vector that points in that direction relative to the camera.
glm::vec3 shootRay()
{
    double x,y;
    glfwGetCursorPos(win, &x, &y);
    glm::vec3 ray = glm::normalize(screenPosToRay(x,y));
    return ray;
}

//takes 4 rays coming from camera and essentially creates a frustum from them. returns the normals of the sides of the frustum.
//this function is used for selecting multiple parts
std::vector<glm::vec3> normalsFromRays(glm::vec3* rays)
{
    std::vector<glm::vec3> result;
    //rays must be in clockwise order
    for(int i = 0; i < 4; i++)
    {
        glm::vec3 one = rays[i], two;
        if(i == 3)
            two = rays[0];
        else
            two = rays[i+1];
        result.push_back(glm::normalize(glm::cross(one,two)));
    }
    return result;
}

//checks if point is inside of a frustum
bool objectInFrustum(std::vector<glm::vec3> frustNorms, glm::vec3 pos)
{
    float camAlongPlaneNorms[4] = {glm::dot(frustNorms[0], camPos), glm::dot(frustNorms[1], camPos), glm::dot(frustNorms[2], camPos), glm::dot(frustNorms[3], camPos)};
    for(int i = 0; i < 4; i++)
    {
        if(camAlongPlaneNorms[i] > glm::dot(frustNorms[i],pos))
            return false;
    }
    return true;
}

//finds the maximum point on a part in a direction
glm::vec3 maxPtInDir(int partIndex, glm::vec3 dir)
{
    PartData& currPart = allParts(partIndex);
    VAOData& currVAO = *(VAOs[pool[partIndex].type]);
    float maxLen = INT_MIN;
    vec3 maxPt;
    for(int i = 0; i < currVAO.partEdges.size(); i += 2)
    {
        int localPtIndex = currVAO.partEdges[i] * 6;
        vec3 localPt = *((vec3*)(currVAO.VBData + localPtIndex));
        vec3 wrldPt = currPart.localToWorld(localPt);
        //take dot product of point in direction, see if this is greater than maxLen
        float dotAmount = glm::dot(wrldPt, dir);
        if(dotAmount > maxLen)
        {
            maxLen = dotAmount;
            maxPt = wrldPt;
        }
    }
    return maxPt;
}


//global variables for dragging parts. stores the intersection point of a ray with the selected part (where we have clicked the selected part)
glm::vec3 intPtClose;
glm::vec3 localIntPtClose;
glm::vec3 intPtCloseNorm;
int closeNormIndex;
glm::vec3 intPtFar;
glm::vec3 intPtFarNorm;
int farNormIndex;

//checks if two parts are colliding, and if the first part is overcolliding with any others
//stores the overcolliding parts in the "touching" vector

#pragma optimize("gty", off)
bool draggedCheckCollide(int partIndex, int farIndex, std::vector<int>& touching, bool requireTouchingFar)
{
    touching.clear();
    bool collisions = false;
    bool touchingFar = false;
    int i = partIndex;
    PartData& part1 = allParts(i);
    PhysicsData& physics1 = allPhys(i);
    unsigned int p1Type = pool[i].type;
    glm::vec3& p1Translate = part1.translate;
    //check for collision with every other part
    float p1Max = std::max(std::max(part1.scaleVector.x, part1.scaleVector.y), part1.scaleVector.z);
    for(int j = 6; j < totalParts; j++)
    {
        // std::cout << "J IS FIRST " << j << '\n';
        if(j < 0 || j > totalParts)
        {
            return false;
        }
        if(j == i)
            continue;
        PartData& part2 = allParts(j);
        PhysicsData& physics2 = allPhys(j);
        unsigned int p2Type = pool[j].type;
        glm::vec3& p2Translate = part2.translate;

        // p1Translate - p2Translate;
        // glm::vec3 hi = operator-(p1Translate,  p2Translate);
        // part1.translate - part2.translate;
        // from1 - from2;
        float distBtw = glm::length(p1Translate - p2Translate);
        float p2Max = std::max(std::max(part2.scaleVector.x, part2.scaleVector.y), part2.scaleVector.z);
        if(distBtw > (p2Max + p1Max + 2.0f))
            //it is not possible for p1 and p2 to intersect
            continue;

        //if we have reached this point, we check for intersection
        glm::vec3 intPts[3];
        // std::cout << "J IS " << j << '\n';
        bool result = SAT(i, j, intPts);
        //if they don't collide, continue
        if(!result)
            continue;
        if(j == farIndex)
        {
            touchingFar = true;
            // std::cout << "TOUCHING FAR\n";
        }
        touching.push_back(j);

        float len = glm::length(intPts[0] - intPts[1]);
        //if the objects don't intersect that much, continue
        if(fGEqual(0.15f, len))
            continue;
        collisions = true;
        touching[touching.size()-1] *= -1;
        // std::cout << "J IS ENDING " << j << '\n';
        // std::cout << "LEN IS " << len << '\n';
    }
    // if(!touchingFar)
    // {
    //     // std::cout << "NOT TOUCHING FAR. Far is: " << farIndex << '\n';
    // }
    // if(collisions)
        // std::cout << "OVERCOLLIDING\n";
    return (!collisions && (touchingFar || !requireTouchingFar));
}

//remove integer from a vector of ints
void removeFromVec(std::vector<int>& vec, int a)
{
    auto it = std::find(vec.begin(), vec.end(), a);
    if(it != vec.end())
    {
        vec.erase(it);
    }
}

//remove a part from the adjacency lists of all the other parts it is touching.
void removeFromAdjacent(int ind)
{
    // std::cout << "START REMOVE FROM ADJ\n";
    std::vector<int>& adj = pool[ind].nodeInfo.adjacent;
    // std::cout << "ADJ SIZE = " << adj.size() << '\n';
    for(int i = 0; i < adj.size(); i++)
    {
        std::vector<int>& adj2 = pool[adj[i]].nodeInfo.adjacent;
        // std::cout << i << '\n';
        removeFromVec(adj2, ind);
    }
    adj.clear();
    // std::cout << "FINISH ADJ\n";
}

//removes a group of parts and all traces of it (though, the parts themselves still exist).
void killGroup(group* groupPtr, bool setMemsToNull)
{
    //set member's groupPtr to NULL
    if(setMemsToNull)
    {
        for(const int& mem : groupPtr->members)
            pool[mem].groupPtr = NULL;
    }
    //search for and delete group
    auto currGroup = groups.begin();
    while(currGroup != groups.end())
    {
        if(&(*currGroup) == groupPtr)
        {
            groups.erase(currGroup);
            break;
        }
        currGroup++; 
    }
    //no need to fix this group since it doesn't exist anymore
    auto it = std::find(groupsToFix.begin(), groupsToFix.end(), groupPtr);
    if(it != groupsToFix.end())
        groupsToFix.erase(it);
}

//removes a part from any group and adjacency lists
void detachPart(int p)
{
    group* groupPtr = pool[p].groupPtr;
    if(groupPtr != NULL)
    {
        // removeFromAdjacent(p);
        // removeFromVec(groupPtr->members, p);
        // if(groupPtr->members.size() == 1)
        //     killGroup(groupPtr, true);
        // else
        // {
        // setGroupMassAndCOM(groupPtr);
        // if(gameMode == UNFREEZE)
        groupsToFix.push_back(groupPtr);
        // }
    }
    removeFromAdjacent(p);
    // pool[p].nodeInfo.adjacent.clear();
    pool[p].groupPtr = NULL;
}

//updates the position of the currently selected part if we are clicking it and dragging the mouse.
void dragParts()
{
    // std::cout << "ASDASDASD " << ctrlHeld << " " << leftClickHeld << " " << selectedParts.size() << '\n';
    if(!ctrlHeld && leftClickHeld && selectedParts.size() == 1 && currTextBox == NULL) 
    {
        if(allPhys(selectedParts[0]).stationary == true)
            return;
        double x,y;
        glfwGetCursorPos(win, &x, &y);
        glm::vec3 ray = screenPosToRay(x,y);
        int farObject = partsRayIntersects(true, ray, camPos, &intPtFar, &farNormIndex, true);
        if(farObject == -1)
            return;
        PartData* farPart = &allParts(farObject);
        VAOData* farVAO = VAOs[pool[farObject].type];
        glm::vec3 farPt = (mat3(farPart->rotationMatrix) * (farPart->scaleVector * farVAO->ptsOnNorms[farNormIndex])) + farPart->translate;
        intPtFarNorm = farPart->applyNM(farVAO->normalsArray[farNormIndex]);

        //now make sure least point of the dragged object along far object normal is touching far point face.

        //first we need to rotate the object
        //find normal that is closest to negative intPtFarNorm, rotate the dragged object so that the face of its norm lines up with intPtFarNorm
        int selectedIndex = selectedParts[0];
        PartData* selectedPart = &allParts(selectedIndex);
        VAOData* selectedVAO = VAOs[pool[selectedIndex].type];
        glm::vec3 closestNorm;
        int normIndex;
        float closestNormDotLength = INT_MIN;
        for(int i = 0; i < selectedVAO->numNormals; i++)
        {
            glm::vec3 currNorm = selectedPart->applyNM(selectedVAO->normalsArray[i]);
            float currDotLength = glm::dot(currNorm, -intPtFarNorm);
            if(currDotLength > closestNormDotLength)
            {
                closestNorm = currNorm;
                closestNormDotLength = currDotLength;
                normIndex = i;
            }
        }
        glm::vec3 rotAxis = glm::cross(closestNorm, -intPtFarNorm);
        //check if rotAxis is equal to 0, if so, the normal we have is perfect and there is no need to rotate, so skill rotation.
        if(!v3Equal(rotAxis, glm::vec3(0,0,0)))
        {
            float rotAmount = acosf(glm::dot(closestNorm, -intPtFarNorm)/(glm::length(closestNorm)*glm::length(-intPtFarNorm)));
            selectedPart->rotate(rotAxis, rotAmount);
        }
        closestNorm = selectedPart->applyNM(selectedVAO->normalsArray[normIndex]);
        //now we need to translate the object so that it is touching the normal of the far object.
        glm::vec3 partMid = selectedPart->translate;
        glm::vec3 ptOnNorm = (mat3(selectedPart->rotationMatrix) * (selectedPart->scaleVector * selectedVAO->ptsOnNorms[normIndex])) + partMid;
        // float normToCenter = glm::dot(closestNorm, (ptOnNorm - partMid));
        // glm::vec3 newTranslateNormDir = ((normToCenter) * intPtFarNorm);

        //now we figure out how much along each axis of the face we can place the object
        glm::vec3 rightFaceVec = glm::cross(glm::vec3(0.0f,1.0f,0.0f), selectedVAO->normalsArray[normIndex]);
        if(v3Equal(rightFaceVec, glm::vec3(0,0,0)))
            rightFaceVec = glm::cross(normalize(glm::vec3(0.1f,1.0f,0.0f)), selectedVAO->normalsArray[normIndex]);
        glm::vec3 upFaceVec = glm::cross(selectedVAO->normalsArray[normIndex], rightFaceVec);
        rightFaceVec = selectedPart->applyNM(rightFaceVec);
        upFaceVec = selectedPart->applyNM(upFaceVec);

        glm::vec3 dragPtClose = selectedPart->localToWorld(localIntPtClose);
        float dragPtDistFromFace = glm::dot(closestNorm, (ptOnNorm - dragPtClose));
        float rayAmountInNormDir = glm::dot(ray, closestNorm);
        float rayMultiplier = dragPtDistFromFace/rayAmountInNormDir;
        glm::vec3 newDragPtPos = (rayMultiplier * -ray) + intPtFar;

        glm::vec3 finalTranslate = newDragPtPos + (partMid - dragPtClose);
        //now we adjust final translate
        float amountUp = 0.25f * (roundf(glm::dot(upFaceVec, finalTranslate)/0.25f));
        float amountRight = 0.25f * (roundf(glm::dot(rightFaceVec, finalTranslate)/0.25f));
        //SUBTRACTING 0.001 so they collide
        finalTranslate = (intPtFarNorm * ((glm::dot(finalTranslate, intPtFarNorm)) - 0.001f)) + amountUp * upFaceVec + amountRight * rightFaceVec;
        // std::cout << " int pt far norm !!!!!!!!!: "; printVec3(intPtFarNorm);
        glm::vec3 oldTranslate = selectedPart->translate;
        selectedPart->translate = finalTranslate;

        //now we must check whether or not the selectedPart now collides a lot with other parts next to it
        //-if so, we move it up until it doesn't collide anymore.
        std::vector<int> touching;
        bool result = draggedCheckCollide(selectedIndex, farObject, touching, true);
        //result is true if the selected part is touching the far object and it is not
        //-over penetrating any other parts
        // std::cout << "RESULT IS: " << result << '\n';
        if(!result || finalTranslate == oldTranslate)
        {
            selectedPart->translate = oldTranslate;
            // std::cout << "dragreturn" << result << '\n';
            return;
        }
        
        
        touching.push_back(selectedIndex);
        
        //remove selected part from the group and adjacency lists it is currently in if any
        
        detachPart(selectedIndex);

        for(int i = 0; i < touching.size(); i++)
        {
            if(allPhys(touching[i]).stationary == true)
                removeFromVec(touching, touching[i]);
        }
        //now we add the objects that are touching to a group together.
        if(touching.size() > 1)
            combineParts(touching, selectedIndex, false);
        // std::cout << "touching final:\n";
        // for(int z = 0; z < touching.size(); z++)
        // {
        //     std::cout << touching[z] << '\n';
        // }
        // std::cout << "DONE COMBINING\n";

        // std::cout << "group nums = " << groups.size() << '\n';
        // std::cout << "GROUP ARE:\n";
        // for(const auto& currGroup: groups)
        // {
        //     for(int i = 0; i < currGroup.members.size(); i++)
        //     {
        //         std::cout << currGroup.members[i] << '\n';
        //     }
        //     std::cout << "next\n";
        // }
    }
}

//returns the closest part that a ray intersects. A ray is basically just a vector with a certain direction that goes on forever.
int partsRayIntersects(bool noScaleParts, glm::vec3 ray, glm::vec3 rayOrigin, glm::vec3* intPtClosest, int* intNormIndex1, bool returnUnselected)
{
    int numIntersect = 0;
    int closestIndex = -1;
    float closestDist = INT_MAX;
    int partsChecking = totalParts;
    int i = noScaleParts ? 6 : 0;
    for(; i < partsChecking; i++)
    {
        //if the ray intersects a scale part, then we are done
        if(i >= 6 && closestIndex <= 5 && closestIndex != -1)
            break;
        float lenToIntPt;
        glm::vec3 intPt;
        int intNormIndex;
        bool result = rayIntersectsPart(i, returnUnselected, rayOrigin, ray, &lenToIntPt, &intPt, &intNormIndex);
        if(result == false)
            continue;
        // std::cout << "After\nASDASDASDSAD";
        PartData* currPart;
        // if(!checkScaleParts) 
        currPart = &allParts(i);

        if(lenToIntPt < closestDist)
        {
            // std::cout << "UPDATED\n";
            if(!returnUnselected || std::find(selectedParts.begin(), selectedParts.end(), i) == selectedParts.end())
            {
                *intPtClosest = intPt;
                
                if(returnUnselected == false)
                {
                    localIntPtClose = (inverse(glm::mat3(currPart->rotationMatrix)) * (intPt - currPart->translate))/currPart->scaleVector;
                    // std::cout << "localIntPtClose is "; printVec3(localIntPtClose);
                }
                closestDist = lenToIntPt;
                closestIndex = i;
                *intNormIndex1 = intNormIndex;
            }
        }
    }
    return closestIndex;
}

//finds whether or not a ray intersects a certain part
bool rayIntersectsPart(int partIndex, bool returnUnselected, glm::vec3 rayOrigin, glm::vec3 ray, float* lenToIntPt, glm::vec3* intPt, int* intNormIndex)
{
    int i;
    int partType;
    glm::vec3* normals;
    std::vector<glm::vec3>* ptsOnNormal;

    // if(!checkScaleParts)
    // {
        i = partIndex;
        partType = pool[i].type;
        normals = VAOs[partType]->normalsArray;
        ptsOnNormal = &VAOs[partType]->ptsOnNorms;
    // }
    // else
    // {
    //     normals = VAOs[GAMECUBE]->normalsArray;
    //     ptsOnNormal = &VAOs[GAMECUBE]->ptsOnNorms;
    // }
    int numNorms = ptsOnNormal->size();
    bool success = false;

    PartData* currPart;
    // if(!checkScaleParts)
        currPart = &allParts(i);

    //index for normals, has it been passed, when if at all ray intersects plane
    std::vector<std::vector<float>> normalData;
    for(int j = 0; j < VAOs[partType]->numNormals; j++)
        normalData.push_back({j * 1.0f, 0, -1.0f});
    //first check which planes we are already in front of
    float planeDistToPoint, planeDistToCam;
    for(int j = 0; j < numNorms; j++)
    {
        glm::vec3 scaleTimesPt;
        glm::vec3 currPoint;
        glm::vec3 currNormal;
        // if(!checkScaleParts)
        // {
            // scaleTimesPt = currPart->scaleVector * (*ptsOnNormal)[j];
            currPoint = (mat3(currPart->rotationMatrix) * (currPart->scaleVector * (*ptsOnNormal)[j])) + currPart->translate;
            currNormal =  currPart->applyNM(normals[j]);
        // }
        // else
        // {
        //     currPoint = (scalePartsScales[i] * (*ptsOnNormal)[j]) + scalePartsPositions[i];
        //     currNormal = normals[j];
        // }
        //if cam is already in front of plane
        planeDistToCam = glm::dot(currNormal, rayOrigin);
        planeDistToPoint = glm::dot(currNormal, currPoint);
        if(planeDistToCam < planeDistToPoint)
            normalData[j][1] = 1;

        //check if ray will ever pass plane
        float distRayMustTravel = planeDistToPoint - planeDistToCam;
        float rayComponentNormalToPlane = glm::dot(currNormal, ray);
        //if ray is not parallel to plane
        if(rayComponentNormalToPlane != 0)
        {
            float lengthOfRayDuringIntersection = distRayMustTravel/rayComponentNormalToPlane;
            // std::cout << "Length of ray during intersection:" << lengthOfRayDuringIntersection << '\n';
            normalData[j][2] = lengthOfRayDuringIntersection;
            float ez = 1;
        }
        else
        {
            normalData[j][2] = FLT_MAX;
        }

    }
    //now check the order in which we intersect
    sort(normalData.begin(), normalData.end(), mousePosSortVecHelper);
    for(int k = 0; k < numNorms; k++)
    {
        std::vector<float> currData = normalData[k];
        if(currData[2] <= 0)
            continue;
        //we pass the plane
        normalData[k][1] = !normalData[k][1];
        int numPlanesInFrontOf = 0;
        for(int z = 0; z < numNorms; z++)
        {
            numPlanesInFrontOf += normalData[z][1];
        }
        if(numPlanesInFrontOf == numNorms)
        {
            success = true;
            glm::vec3 intPt1 = rayOrigin + (ray * normalData[k][2]);
            *intPt = intPt1;
            float lenToIntPt1 = glm::length(intPt1 - rayOrigin);
            *lenToIntPt = lenToIntPt1;
            *intNormIndex = (int)(currData[0]);
            break;
        }   
    }
    return success;
}

//changes the screen position of the frustum we are using to currently select multiple parts (with ctrl + click).
void updateSelectVerts()
{
    glm::vec2 s, e;
    multSelectPositions[0] = s = startSelection;
    
    double x,y;
    glfwGetCursorPos(win, &x, &y);
    multSelectPositions[2] = e = glm::vec2(x, y);
    glm::vec2 pos2, pos4;
    //start and end in bottom left and top right
    if((s[0]<e[0] && s[1]>e[1])||(s[0]>e[0] && s[1]<e[1])) {
        pos2 = glm::vec2(s[0],e[1]);
        pos4 = glm::vec2(e[0],s[1]);
    }
    //start and end in bottom right and top left
    else {
        pos2 = glm::vec2(e[0],s[1]);
        pos4 = glm::vec2(s[0],e[1]);
    }
    multSelectPositions[1] = pos2; multSelectPositions[3] = pos4;
    //update the vertices of the rectangular selection the user is making
    for(int i = 0; i < 4; i++)
    {
        glm::vec2 curr = multSelectPositions[i];
        float xFromCenter = ((curr[0] - (pixelWidth/2.0f))/(pixelWidth/2.0f));
        float yFromCenter = -((curr[1] - (pixelHeight/2.0f))/(pixelHeight/2.0f));
        multSelectVerts[i] = glm::vec3(xFromCenter, yFromCenter, 0.0f);
    }
}

//checks if a button (on screen) is being clicked
bool buttonPressed(buttonData& currButton, float x, float y, bool inMenu)
{
    bool pressed = false;
    if(currButton.visible == false)
        return pressed;

    if(inMenu)
    {
        float newWidth = menuRight - menuLeft, oldWidth = pixelWidth;
        float newHeight = menuTop - menuBottom, oldHeight = pixelHeight;
        x *= oldWidth/newWidth;
        y *= oldHeight/newHeight;
    }
    vec2 pos = currButton.position;
    vec2 scale = currButton.scale;
    // float leftX = pos.x - (0.5f * scale.x), rightX = pos.x + (0.5f * scale.x);
    // float topY = pos.y + (0.5f * scale.y), bottomY = pos.y - (0.5f * scale.y);

    float leftX = pos.x, rightX = pos.x + (scale.x);
    float topY = pos.y + (scale.y), bottomY = pos.y;


    //now we check if mouse is inside this button
    if(x >= leftX && x <= rightX && y <= topY && y >= bottomY)
    {
        pressed = true;
        std::cout << "button pressed.\n";
        currButton.onClick(currButton);
    }
    return pressed;
}

//checks if any of the buttons on screen are pressed. if they are, call their "onClick" function.
bool buttonsPressed()
{
    bool pressed = false;
    double x,y;
    glfwGetCursorPos(win, &x, &y);
    mouseX = x;
    mouseY = pixelHeight - y;
    std::cout << mouseX << " mouseX\n";
    std::cout << mouseY << " mouseY\n";
    int start = 0, end = 0;
    switch(gameState)
    {
        case PLAYING:
            start = 0; end = 2;
            pressed = pressed || buttonPressed(colorButton, mouseX, mouseY, false);
            break;
        case SAVEMENU:
            start = 3; end = 6;
            for(int i = 0; i < nameButtons.size(); i++)
            {
                pressed = pressed || buttonPressed(nameButtons[i], mouseX, mouseY, gameState == SAVEMENU);
            }
            break;
        case TITLESCREEN:
            start = 7; end = 9;
            break;
        case INSTRUCTIONS:
            start = 10; end = 10;
    }
    for(int i = start; i <= end; i++)
        pressed = pressed || buttonPressed(buttons[i], mouseX, mouseY, gameState == SAVEMENU);

    glViewport(0, 0, pixelWidth, pixelHeight);
    
    return pressed;
}

//globals for whether or not we are dragging the scaling parts, and if so, which one we are dragging
bool draggingScalePart = false;
int scalePartInd = -1;
bool scaleSelected = true;

float newPrecision(float x, int precision)
{
    return( truncf(x * pow(10, precision))/(pow(10, precision)));
}

std::string vec3ToString(vec3 v)
{
    v = vec3(newPrecision(v.r,1),newPrecision(v.g,1),newPrecision(v.b,1));
    std::string vecStrs[] = {to_string(v.r).substr(0,3),to_string(v.g).substr(0,3),to_string(v.b).substr(0,3)};
    std::string result = vecStrs[0] + "," + vecStrs[1] + "," + vecStrs[2];
    return result;
}

//checks for mouse input (left and right buttons)
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if(action == GLFW_PRESS)
        {
            rightPressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // std::cout << "left pressed\n";
        }
        else if(action == GLFW_RELEASE)
        {
            rightPressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            // std::cout << "left released\n";
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            if(creatingTB)
            {
                currTextBox = NULL;
                creatingTB = false;
                saves.pop_back();
                nameButtons.pop_back();
            }
            colorTB = false;
            bool pressed = buttonsPressed();
            std::cout << "Pressed\n";
            leftClickHeld = true;
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            mouseX = x; mouseY = y;
            if(gameState != PLAYING)
                return;
            //toggle selecting multiple parts
            if(ctrlHeld)
            {
                multipleSelect = true;
                startSelection = glm::vec2(mouseX, mouseY);
            }
            else
            {
                if(pressed)
                    return;
                glm::vec3 ray = shootRay();

                //first we check if we are clicking any parts used for scaling, which take priority over all others.

                int closestIndex = partsRayIntersects(false, ray, camPos, &intPtClose, &closeNormIndex, false);
                if(closestIndex != -1 && closestIndex != 6)
                {
                    glfwSetCursor(window, cursor);
                    if(closestIndex < 6)
                    {
                        //we are clicking a scale part
                        scalePartInd = closestIndex;
                        draggingScalePart = true;
                    }
                    else
                    {
                        //if the thing we clicked is not selected clear the selection and add the new thing to it.
                        if(std::find(selectedParts.begin(), selectedParts.end(), closestIndex) == selectedParts.end())
                        {
                            selectedParts.clear();
                            selectedParts.push_back(closestIndex);
                            colorButton.text = vec3ToString(allParts(selectedParts[0]).color);
                        }

                        if(currTextBox != NULL && currTextBox == &colorButton)
                            currTextBox->text = vec3ToString(allParts(selectedParts[0]).color);
                        currTextBox = NULL;
                    }
                }
                else
                {
                    scalePartsDisappear();
                    selectedParts.clear();
                    colorButton.text = "COLOR";
                    currTextBox = NULL;
                }
                //can now drag selected parts
                
            }
        }
        else if(action == GLFW_RELEASE)
        {
            glfwSetCursor(window, NULL);
            draggingScalePart = false;
            leftClickHeld = false;
            //now we must calculate the vector pointing towards where we clicked
            if(multipleSelect)
            {
                
                
                selectedParts.clear();
                glm::vec2* msv = multSelectPositions;
                glm::vec2 s = msv[0], pos2 = msv[1], e = msv[2], pos4 = msv[3];
                glm::vec3 rays[4] = {screenPosToRay(s[0],s[1]),screenPosToRay(pos2[0],pos2[1]),
                                    screenPosToRay(e[0],e[1]), screenPosToRay(pos4[0],pos4[1])};
                std::vector<glm::vec3> norms = normalsFromRays(rays);
                int numInFrustum = 0;
                for(int i = 7; i < totalParts; i++)
                {
                    PartData* currPart = &allParts(i);
                    bool result = objectInFrustum(norms,currPart->translate);
                    //mark object as selecting by adding it to selected parts vector
                    if(result)
                    {
                        numInFrustum++;
                        selectedParts.push_back(i);
                    }
                }
                if(numInFrustum > 1)
                {
                    colorButton.text = "";
                }
                else if(numInFrustum == 1)
                {
                    colorButton.text = vec3ToString(allParts(selectedParts[0]).color);
                }
                // std::cout << "numin:" << numInFrustum << '\n';
            }
            multipleSelect = false;
            std::cout << "multselectVerts:\n";
            for(vec3 ok : multSelectVerts)
            {
                printVec3(ok); std::cout << "\n";
            }

        }
    }
}

//if we have selected a text box (keyboard input must be routed there), then this global will not be NULL.
buttonData* currTextBox = NULL;
bool colorTB = false;

//convert a bunch of characters into a vector. This function is used for converting an RGB value a user types into a vec3.
glm::vec3 stringToVec3(std::string str, bool& success)
{
    success = true;
    size_t numChars;
    stringstream stream(str);
    vector<string> result;

    while(stream.good())
    {
        string substr;
        getline(stream, substr, ',');
        result.push_back(substr);
    }
    if(result.size() != 3)
    {
        std::cout << "ERROR, wrong number of numbers.\n";
        success = false;
        return vec3(0,0,0);
    }
    vec3 resultVec;
    for(int i = 0; i < 3; i++)
    {
        try
        {
            resultVec[i] = stof(result[i], &numChars);  
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            success = false;
            return vec3(0,0,0);
            break;

        }
        
    }
    return resultVec;

}

int scrollAmount = 0;

//if we are in a state where it is appropriate, scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(gameState == SAVEMENU || gameState == INSTRUCTIONS)
    {
        scrollAmount += yoffset * -20;
        if(scrollAmount < 0)
            scrollAmount = 0;
        std::cout << scrollAmount << " : scrollAmount\n";
    }
}

void toggleScaleOrMove()
{
    scaleSelected = !scaleSelected;
    for(int i = 0; i < 6; i++)
        allParts(i).color = (float)scaleSelected * vec3(0.0f,0.0f,1.0f) + (float)!scaleSelected * vec3(0.5f,0.5f,0.0f);
}

//checks if user has pressed a button on the keyboard
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        //if text box is toggled, route input there, then return.
        if(currTextBox != NULL)
        {
            if(key >= 0 && key <= 127 && (currTextBox->text.size() < 6 || (currTextBox->text.size() < 11 && colorTB)))
                currTextBox->text += char(key);
            else if(key == GLFW_KEY_BACKSPACE && (currTextBox->text.size() != 0))
                currTextBox->text.pop_back();
            else if(key == GLFW_KEY_ENTER)
            {
                if(colorTB)
                {
                    bool success;
                    vec3 newColor = stringToVec3(colorButton.text, success);
                    if(success)
                    {
                        currNewColor = newColor;
                        for(int i = 0; i < selectedParts.size(); i++)
                        {
                            allParts(selectedParts[i]).color = newColor;
                        }
                    }
                    else
                        colorButton.text = vec3ToString(allParts(selectedParts[0]).color);
                    colorTB = false;
                    currTextBox = NULL;
                }
                else
                {
                    if(std::find(saves.begin(), saves.end(), currTextBox->text) != saves.end())
                    {
                        //not done yet, we can't create a new save with the same name as an old one
                        std::cout << "Can't create new save with duplicate name\n";
                    }
                    else
                    {
                        currTextBox = NULL;
                        creatingTB = false;
                    }
                }
                
            }
            return;
        }
        //if no text box is selected, do something specific depending on the key pressed
        switch(key)
        {
            //the first 4 cases are for moving the camera around
            case GLFW_KEY_W:
                movement[0] = 1;
                break;
            case GLFW_KEY_S:
                movement[1] = 1;
                break;
            case GLFW_KEY_D:
                movement[2] = 1;
                break;
            case GLFW_KEY_A:
                movement[3] = 1;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                if(!leftClickHeld)
                    ctrlHeld = true;
                break;
            case GLFW_KEY_R:
            case GLFW_KEY_T:
                rotateSelected(key);
                break;
            case GLFW_KEY_BACKSPACE:
                deleteParts();
                break;
            case GLFW_KEY_F:
                addPart();
                break;
            case GLFW_KEY_L:
                if(gameState != PLAYING && gameState != SAVEMENU)
                    break;
                renderSaveMenu = !renderSaveMenu;
                gameState = PLAYING * !renderSaveMenu + SAVEMENU * renderSaveMenu;
                scrollAmount = 0;
                break;
            case GLFW_KEY_ESCAPE:
                renderSaveMenu = false;
                renderTitleScreen = true;
                scrollAmount = 0;
                gameState = TITLESCREEN;
                break;
            case GLFW_KEY_Q:
                toggleScaleOrMove();
                break;
            //copies parts
            case GLFW_KEY_C:
                if(selectedParts.size() > 0)
                    copyParts();
                break;
            //pastes parts
            case GLFW_KEY_V:
                if(partsToCopy.size() > 0)
                    pasteParts();
                break;
            //creates an explosion
            case GLFW_KEY_Z:
                // shootRay
                vec3 ray = shootRay();
                vec3 explosionPt; int closeNormExplosion;
                int closestIndex = partsRayIntersects(false, ray, camPos, &explosionPt, &closeNormExplosion, false);
                if(closestIndex != -1)
                    createExplosion(explosionPt, 20.0f, 1.0f);
                startTiming = true;
                break;
           
        }
    }
    if(action == GLFW_RELEASE)
    {
        switch(key)
        {
            //stop moving camera 
            case GLFW_KEY_W:
                movement[0] = 0;
                break;
            case GLFW_KEY_S:
                movement[1] = 0;
                break;
            case GLFW_KEY_D:
                movement[2] = 0;
                break;
            case GLFW_KEY_A:
                movement[3] = 0;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                ctrlHeld = false;
        }
    }
}

//rotate or tilt the currently selected part
void rotateSelected(int key)
{
    glm::vec3 rotVec;
    if(key == GLFW_KEY_R)
        rotVec = glm::vec3(0.0f,1.0f,0.0f);
    else if(key == GLFW_KEY_T)
        rotVec = glm::vec3(1.0f,0.0f,0.0f);
    else
        return;
    
    for(int i = 0; i < selectedParts.size(); i++)
    {
        PartData* currSelected = &allParts(selectedParts[i]);
        glm::vec3 currRotVec = currSelected->applyNM(rotVec);
        currSelected->rotate(currRotVec, glm::radians(90.0f));
    }
}


//move the camera
void move()
{
    //ignore y component of camFront so we don't move in the y direction
    //move forward and back
    camPos += camFront * (speed * (movement[0] - movement[1]));
    //move right and left
    camPos += camRight * (speed * (movement[2] - movement[3]));
}

//changing the size of the window
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if(width == 0 || height == 0)
    {
        return;
    }
    glViewport(0, 0, width, height);
    // return;
    pixelWidth = width;
    pixelHeight = height;
    aspectRatio = pixelWidth/pixelHeight;


    //change the fov and the size of the projection frustum
    fovY = glm::degrees(2 * atan((pixelHeight/2.0f)/depth));

    proj = glm::perspective(glm::radians(fovY), aspectRatio, 0.1f, 100.0f);
    orthoProj = glm::ortho(0.0f, pixelWidth, 0.0f, pixelHeight);
    menuTop = ((saveMenuBox.position.y + (0.5f * saveMenuBox.scale.y)) * pixelHeight/2.0f) + pixelHeight/2.0f;
    menuLeft = ((saveMenuBox.position.x + (-0.5f * saveMenuBox.scale.x)) * pixelWidth/2.0f) + pixelWidth/2.0f;
    std::cout << menuLeft << " menu Left\n";
    menuRight = pixelWidth - menuLeft;
    menuBottom = pixelHeight - menuTop;
    saveProj = glm::ortho(menuLeft, menuRight, menuBottom, menuTop);
    // //calculating fovX
    // //the 1.0f below is the opposite side, it is a redundant number
    // float hY = 1.0f/sin(glm::radians(fovY/2));
    // float oX = aspectRatio * 1.0f;
    // // return;
    // std::cout << "hY: " << hY << " oX: " << oX << '\n';
    //hY and hX are the same
    fovX = glm::degrees(2 * atanf(tanf(glm::radians(fovY/2.0f)) * aspectRatio));
    // std::cout << "oX/hY is " << oX/hY << '\n';
}


//sends updated part data to gpu and renders parts.
void updateAndDraw(bool selected, bool outline, bool drawTransformationParts)
{
    //first we must scale up selected parts and eventually make them pink. This will be their outline
    if(selected && outline) 
    {
        for(int i = 0; i < selectedParts.size(); i++) 
        {
            PartData* currPart = &allParts(selectedParts[i]);
            currPart->scale(currPart->scaleVector + glm::vec3(0.1f,0.1f,0.1f));
            if(pool[selectedParts[i]].type == GAMESLANT)
                currPart->translate += glm::vec3(0.0f,0.025f,0.0f);
        }
    }
    for(int k = 0; k < numVAOs; k++)
        VAOs[k]->updateParts(true, 0, 0);
    for(int k = 0; k < numVAOs; k++)
        VAOs[k]->drawParts(selected, outline, drawTransformationParts);
    if(selected && outline) 
    {
        for(int i = 0; i < selectedParts.size(); i++) 
        {
            PartData* currPart = &allParts(selectedParts[i]);
            currPart->scale(currPart->scaleVector + glm::vec3(-0.1f,-0.1f,-0.1f));
            if(pool[selectedParts[i]].type == GAMESLANT)
                currPart->translate -= glm::vec3(0.0f,0.025f,0.0f);
        }
    }
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if(!glfwInit())
    {
        std::cout << "CAN'T INITIALIZE GLFW";
    }
    //window settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    win = glfwCreateWindow(640, 480, "My game", NULL, NULL);
    
    //callbacks - tells glfw what to do with input from user
    glfwSetKeyCallback(win, key_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetCursorPosCallback(win, mousePosCallback);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    if(!win)
    {
        std::cout << "CAN'T CREATE GLFW WINDOW";
    }
    glfwMakeContextCurrent(win);
    //initialize glew here
    GLenum error = glewInit();
    if(error != GLEW_OK)
    {
        std::cout << "Error: " << glewGetErrorString(error) << '\n';
    }
    glViewport(0,0, 640, 480);
    
    //set texture settings
    stbi_set_flip_vertically_on_load(true);
    int hello = 1;
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);  
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);
    frame1Time = glfwGetTime();
    frame2Time = frame1Time;

    initializeBuiltInParts();
    std::cout << "FINISH BUILT IN\n";
    initButtons();
    std::cout << "START scalepart\n";
    createScaleParts();
    
    int i;
    int cubeIndex = totalParts;
    cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    
    std::cout << "b\n";

    int pt = GAMECUBE;
    int c = 6;
    //this loop creates the initial parts in the scene
    for(i = 0; i < 3; i++)
    {
        std::cout << "a\n";
        PartData currPart = *(VAOs[pt]->addPart());
        int num = VAOs[pt]->numParts - 1;
        VAOs[pt]->parts[num].color = glm::vec3(std::rand()/(RAND_MAX * 1.0f), std::rand()/(RAND_MAX * 1.0f),std::rand()/(RAND_MAX * 1.0f));   
        VAOs[pt]->parts[num].translate = glm::vec3(0.0f,0.0f,0.0f);
        std::cout << "c\n";
    }

    VAOs[pt]->parts[0+c].translate = glm::vec3(0.0f,-2.0f,0.0f);
    VAOs[pt]->parts[0+c].scale({1000.0f,1.0f,1000.0f}); 
    VAOs[pt]->parts[1+c].scale({1.0f,1.0f,10.0f}); 
    VAOs[pt]->parts[1+c].translate = vec3(2.0f,2.0f,4.0f);
    VAOs[pt]->parts[2+c].rotate(vec3(1.0f,1.0f,1.0f), glm::radians(30.0f));
    VAOs[pt]->partsPhysics[0+c].stationary = true;

    VAOs[pt]->partsPhysics[0+c].mass = 300.0f;
    VAOs[pt]->parts[0+c].color = vec3(0.0f,0.5f,0.0f);

    int test1 = true;

    glm::vec3 screenPos{0.0f,0.0f,1.0f};
    
    std::cout << "hello\n";
    glm::vec3 squarePts[4] = 
    {
        {0.5f,0.5f,-0.5f},
        {0.5f,-0.5f,-0.5f},
        {-0.5f,-0.5f,-0.5f},
        {-0.5f,0.5f,-0.5f}
    };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setupFont();
    getOldSaves();
    std::cout << "START DRAWING\n";

    while(!glfwWindowShouldClose(win))
    {
        //game loop
        glfwPollEvents();
        if(glfwGetWindowAttrib(win, GLFW_ICONIFIED))
        {
            continue;
        }
        frame1Time = frame2Time;
        frame2Time = glfwGetTime();
        deltaTime = frame2Time - frame1Time;
        speed = 5.0f * deltaTime;
        
        if(selectedParts.size() != 0)
        {
            updateScaleParts();
        }

        fixGroups();

        
        move();
        
        //view matrix (transform world to camera/view coords)
        view = glm::mat4(1.0f);
        view = glm::lookAt(camPos, camPos + camFront, camUp);
        
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glDisable(GL_STENCIL_TEST);
        //draw correct screen based on gameState
        int hi = VAOs[GAMECUBE]->numParts;
        int hi2 = VAOs[GAMECUBE]->parts.size();
        switch(gameState)
        {
            case PLAYING:
                glEnable(GL_DEPTH_TEST);
                glStencilMask(0xFF); 
                glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                glDisable(GL_STENCIL_TEST);

                updateAndDraw(false, false, false);
                
                glEnable(GL_STENCIL_TEST);
                glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE); 
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF); 

                updateAndDraw(true, false, false);
                
                glDisable(GL_DEPTH_TEST);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                updateAndDraw(true, true, false);

                
                glDisable(GL_STENCIL_TEST);
                // glEnable(GL_DEPTH_TEST);
                // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                // drawPoint(rayIntPt, glm::vec3(0.0f,1.0f,0.0f));
                
                updateAndDraw(false, false, true);
                drawSceneButtons();
                break;
            case SAVEMENU:
                drawSaveMenu();
                break;
            case TITLESCREEN:
                drawTitleScreen();
                break;
            case INSTRUCTIONS:
                drawInstructionMenu();
                break;
        }
        if(multipleSelect)
        {
            updateSelectVerts();
            drawSquareFrame(multSelectVerts);
        }
        glfwSwapBuffers(win);
        if(gameState == PLAYING && gameMode == UNFREEZE)
            physicsTick();

    }
    glfwDestroyWindow(win);
    glfwTerminate();
    std::cout << "main done\n";
}