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
#include <algorithm>

float pixelWidth = 640.0f;
float pixelHeight = 480.0f;
float aspectRatio = pixelWidth/pixelHeight;

float fovY = 45.0f;
float fovX = glm::degrees(asinf(sin(glm::radians(fovY/2)) * aspectRatio) * 2);
glm::mat4 view;
glm::mat4 proj = glm::perspective(glm::radians(fovY), aspectRatio, 0.1f, 100.0f);

float mixNum = 0.5f;
unsigned int shaderProgram;
float frame1Time, frame2Time, deltaTime;
glm::vec3 camPos{0.0f, 0.0f, 6.0f};
glm::vec3 camFront{0.0f, 0.0f, -1.0f};
glm::vec3 camUp{0.0f, 1.0f, 0.0f};
glm::vec3 camRight;
glm::vec3 camUpReal = glm::cross(camRight, camFront);

short movement[4];
float speed;
float lastX, lastY;
float yaw = 270.0f, pitch = 0;
bool leftPressed = false;
bool leftPressedLastMove = false;

int mouseX;
int mouseY;

bool go1 = false, go2 = false;
void error_callback(int error, const char* description)
{
    std::cout << "ERROR " << error << '\n' << description << '\n';
}

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

void mousePosCallback(GLFWwindow* window, double x, double y)
{
    //only rotate camera if left mouse button is held
    if(!leftPressed)
    {
        leftPressedLastMove = false;
        return;
    }
    if(leftPressed && !leftPressedLastMove)
    {
        leftPressedLastMove = true;
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
        float changeX = x - lastX, changeY = y - lastY;
        yaw += changeX * 0.25;
        pitch -= changeY * 0.25;
        if(pitch <= -90.0f)
            pitch = -89.5f;
        if(pitch >= 90.0f)
            pitch = 89.5f;
        if(yaw >= 360.0f)
            yaw = 0.0f;
        if(yaw < 0.0f)
            yaw = 360.0f;
        camFront = yawPitchToVec(yaw, pitch);
        camRight = glm::normalize(glm::cross(camFront, camUp));
        camUpReal = glm::normalize(glm::cross(camRight, camFront));
        // std::cout << "yaw = " << yaw << " pitch = " << pitch << '\n';
        lastX = x;
        lastY = y;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if(action == GLFW_PRESS)
        {
            leftPressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // std::cout << "left pressed\n";
        }
        else if(action == GLFW_RELEASE)
        {
            leftPressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            // std::cout << "left released\n";
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_RELEASE)
        {
            //now we must calculate the vector pointing towards where we clicked
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            mouseX = x; mouseY = y;
            //x and y from center range from 0 to 1.0f.
            float xFromCenter = ((mouseX - (pixelWidth/2.0f))/(pixelWidth/2.0f)) * sin(glm::radians((fovX/2)));
            float yFromCenter = ((mouseY - (pixelHeight/2.0f))/(pixelHeight/2.0f)) * sin(glm::radians((fovY/2)));
            float addYaw = asin(xFromCenter);
            float addPitch = asin(yFromCenter);
            // std::cout << "addYaw: " << addYaw << " addPitch " << addPitch << '\n';
            glm::mat4 rotMat = glm::rotate(glm::rotate(glm::mat4(1.0f), addPitch, -camRight), addYaw, -camUpReal);
            glm::vec3 ray = rotMat * glm::vec4(camFront, 1.0f);
            std::cout << "ray x: " << ray.x << " ray y: " << ray.y << " ray z: " << ray.z << '\n';
            //check if ray intersects

            //calculate plane normals with cross product    
            glm::vec3* normals = VAOs[GAMECUBE]->normalsArray;

            GLfloat* pointsOnPlanes = VAOs[GAMECUBE]->VBData;
            int numIntersect = 0;
            for(int i = 0; i < VAOs[GAMECUBE]->numParts; i++)
            {
                bool success = false;

                PartData* currPart = &(VAOs[GAMECUBE]->parts[i]);
                //index for normals, has it been passed, when if at all ray intersects plane

                std::vector<std::vector<float>> normalData= {{0, 0, -1.0f}, {1, 0, -1.0f}, {2, 0, -1.0f}, {3, 0, -1}, {4, 0, -1}, {5, 0, -1}};

                //first check which planes we are already in front of
                float planeDistToPoint, planeDistToCam;
                for(int j = 0; j < 6; j++)
                {
                    glm::vec3 currPoint = currPart->normalMatrix * (*(glm::vec3*)(pointsOnPlanes + (6*5*j))) + currPart->translate;
                    glm::vec3 currNormal =  currPart->normalMatrix * normals[j];
                    //if cam is already in front of plane
                    planeDistToCam = glm::dot(currNormal, camPos);
                    planeDistToPoint = glm::dot(currNormal, currPoint);
                    if(planeDistToCam > planeDistToPoint)
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
                    }

                }

                //now check the order in which we intersect
                sort(normalData.begin(), normalData.end(), mousePosSortVecHelper);
                for(int k = 0; k < 6; k++)
                {
                    std::vector<float> currData = normalData[k];
                    if(currData[2] <= 0)
                        continue;
                    //we pass the plane
                    normalData[k][1] = !normalData[k][1];
                    int numPlanesInFrontOf = 0;
                    for(int z = 0; z < 6; z++)
                    {
                        numPlanesInFrontOf += normalData[z][1];
                    }
                    if(numPlanesInFrontOf == 6)
                    {
                        success = true;
                        numIntersect += 1;
                        break;
                    }
                }
            }
            std::cout << "num of objects intersected by ray: " << numIntersect << '\n';
            // std::cout << "ray X: " << ray.x << " ray Y: " << ray.y << " ray Z: " << ray.z << '\n';
            
            // new frustum culling method
            // check if every vertex of a part doesn't go past a plane of the frustum. If this is true
            // .. for even just one plane, the object is not inside the frustum.

            // first we must get the normals of all the planes (4 of them if you don't count the "far" plane)
            
            
            glm::vec3 rightPlane, leftPlane, topPlane, bottomPlane;

            //NOTE: negative pitch is actually going up
            rightPlane = yawPitchToVec(yaw + fovX/2 - 90, pitch);
            leftPlane = yawPitchToVec(yaw + 90 - fovX/2, pitch);
            topPlane = yawPitchToVec(yaw, pitch - fovY/2 + 90);
            bottomPlane = yawPitchToVec(yaw, pitch + fovY/2 - 90);
            std::cout << "fovX is " << fovX << '\n';
            // std::cout << "top plane is " << "x: " << topPlane.x << " y: " << topPlane.y << " z: " << topPlane.z << '\n';
            glm::vec3 planes[4] = {rightPlane, leftPlane, topPlane, bottomPlane};
            float camAlongPlaneNormals[4] = {glm::dot(rightPlane, camPos), glm::dot(leftPlane, camPos), glm::dot(topPlane, camPos), glm::dot(bottomPlane, camPos)};
            std::string names[4] = {"right plane", "left plane", "top plane", "bottom plane"};
            //now we check how far in the directions of the planes the vertices are.
            int numInView = 0;
            int numCubeVerts = VAOs[GAMECUBE]->VBDataSize/(sizeof(GLfloat)*5);
            //for every part
            for(int i = 0; i < VAOs[GAMECUBE]->numParts; i++)
            {
                // std::cout << "fovx is " << fovX << '\n';
                PartData* currPart = &(VAOs[GAMECUBE]->parts[i]);
                glm::mat4 model = currPart->rotationMatrix;
                model[3] = glm::vec4(currPart->translate, 1.0f);

                bool inView = true;
                //for every plane
                for(int j = 0; j < 4; j++)
                {
                    int verticesOutsidePlane = 0;
                    //for every unique vertex in the current part
                    for(int k = 0; k < numCubeVerts; k++)
                    {
                        GLfloat* currVertStart = VAOs[GAMECUBE]->VBData + (k * 5);
                        glm::vec3 currVert = model * glm::vec4(currVertStart[0], currVertStart[1], currVertStart[2], 1.0f);
                        float amountInPlaneDir = glm::dot(planes[j], currVert);
                        verticesOutsidePlane += (camAlongPlaneNormals[j] > amountInPlaneDir);
                    }
                    if(verticesOutsidePlane == numCubeVerts)
                    {
                        inView = false;
                        break;
                    }
                }
                numInView += inView;
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
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
            case GLFW_KEY_Z:
                go1 = true;
                go2 = false;
                break;
            case GLFW_KEY_X:
                go1 = false;
                go2 = true;
                break;
        }
    }
    if(action == GLFW_RELEASE)
    {
        switch(key)
        {
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
        }
    }
}
void move()
{
    //ignore y component of camFront so we don't move in the y direction
    // glm::vec3 camFrontXZ = glm::normalize(glm::vec3(camFront.x, 0.0f, camFront.z));
    // //move forward and back
    // camPos += camFrontXZ * (speed * (movement[0] - movement[1]));
    // //move right and left
    // camPos += glm::normalize(glm::cross(camFrontXZ, camUp)) * (speed * (movement[2] - movement[3]));

    //ignore y component of camFront so we don't move in the y direction
    //move forward and back
    camPos += camFront * (speed * (movement[0] - movement[1]));
    //move right and left
    camPos += camRight * (speed * (movement[2] - movement[3]));

}
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    pixelWidth = width;
    pixelHeight = height;
    aspectRatio = pixelWidth/pixelHeight;
    proj = glm::perspective(glm::radians(fovY), aspectRatio, 0.1f, 100.0f);

    //calculating fovX
    //the 1.0f below is the opposite side, it is a redundant number
    float hY = 1.0f/sin(glm::radians(fovY/2));
    float oX = aspectRatio * 1.0f;
    //hY and hX are the same
    fovX = glm::degrees(asinf(oX/hY) * 2);
    std::cout << "oX/hY is " << oX/hY << '\n';
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
    GLFWwindow* window = glfwCreateWindow(640, 480, "My game", NULL, NULL);
    
    //callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    if(!window)
    {
        std::cout << "CAN'T CREATE GLFW WINDOW";
    }
    glfwMakeContextCurrent(window);
    //initialize glew here
    GLenum error = glewInit();
    if(error != GLEW_OK)
    {
        std::cout << "Error: " << glewGetErrorString(error) << '\n';
    }
    glViewport(0,0, 640, 480);
    
    glUseProgram(shaderProgram);

    //texture stuff
    stbi_set_flip_vertically_on_load(true);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);  
    glEnable(GL_DEPTH_TEST);  
    glfwSwapInterval(1);
    frame1Time = glfwGetTime();
    frame2Time = frame1Time;
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    initializeBuiltInParts();
    // VAOs[GAMECUBE]->addPart();
    // VAOs[GAMECUBE]->addPart();
    // VAOs[GAMECUBE]->addPart();
    // VAOs[GAMECUBE]->addPart();
    // VAOs[GAMECUBE]->parts[0].rotate(glm::vec3(0.0f,0.0f,1.0f), 45);
    // VAOs[GAMECUBE]->parts[0].translate = glm::vec3(0.0f,0.0f,6.0f);
    // VAOs[GAMECUBE]->parts[1].translate = glm::vec3(6.0f,0.0f,0.0f);
    // VAOs[GAMECUBE]->parts[2].translate = glm::vec3(1.0f,0.0f,0.0f);
    PartData* myPart = VAOs[GAMECYLINDER]->addPart();
    myPart->color = glm::vec3(1.0f, 0.0f, 0.0f);
    myPart->scale(glm::vec3(1.0f, 2.0f, 1.0f));
    int i;
    int startNum = VAOs[GAMECUBE]->numParts;
    for(i = 0; i < 2; i++)
    {
        VAOs[GAMECUBE]->addPart();
        int num = VAOs[GAMECUBE]->numParts - 1;
        // VAOs[GAMECUBE]->parts[num].rotationMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(8.0f, 1.0f, 1.0f));
        // VAOs[GAMECUBE]->parts[num].translate = glm::vec3(50.0f * (std::rand()/(RAND_MAX * 1.0f)), 50.0f * (std::rand()/(RAND_MAX * 1.0f)), 50.0f * (std::rand()/(RAND_MAX * 1.0f)));
        VAOs[GAMECUBE]->parts[num].color = glm::vec3(std::rand()/(RAND_MAX * 1.0f), std::rand()/(RAND_MAX * 1.0f),std::rand()/(RAND_MAX * 1.0f));    
    }
    VAOs[GAMECYLINDER]->parts[1].translate = glm::vec3(0.0f, 0.0f, 4.0f);
    // cylinderIntersect();
    while(!glfwWindowShouldClose(window))
    {
        //game loop i think
        
        frame1Time = frame2Time;
        frame2Time = glfwGetTime();
        deltaTime = frame2Time - frame1Time;
        speed = 5.0f * deltaTime;

        glfwPollEvents();

        move();

        VAOs[GAMECUBE]->parts[0].translate += glm::vec3(0.0f, 0.0f, 0.01f * go1 - 0.01f * go2);
        //view matrix (transform world to camera/view coords)
        view = glm::mat4(1.0f);
        view = glm::lookAt(camPos, camPos + camFront, camUp);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

        glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), camPos.x, camPos.y, camPos.z);
        float time = 30 * glfwGetTime();
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 5.0f * cos(glm::radians(time)), 0.0f, -5.0f + (5.0f * sin(glm::radians(time))));
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(int k = 0; k < numVAOs; k++)
            VAOs[k]->updateParts(true, 0, 0);
        for(int k = 0; k < numVAOs; k++)
            VAOs[k]->drawParts();

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}