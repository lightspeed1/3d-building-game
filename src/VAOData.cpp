#include <deque>
#include <iostream>
#include "include/VAOData.h"
#include "include/ShaderSources.h"
#include "include/game.h"
#include "include/building.h"
# define M_PI 3.14159265358979323846

using namespace glm;
using namespace std;


// static GLfloat cubeVBData[] = {
//     //face1
//     //bottom left
//     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
//     //br
//     0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
//     //tr
//     0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
//     //tl
//     -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
//     //face2
//     //bottom left
//     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
//     //br
//     0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
//     //tr
//     0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
//     //tl
//     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
// };

// static GLuint cubeEBData[] = {
//     //front
//     0, 1, 2,
//     0, 2, 3,
//     //back
//     6, 5, 4,
//     7, 6, 4,

//     //right
//     5, 2, 1,
//     5, 6, 2,

//     //left
//     0, 3, 4,
//     3, 7, 4,

//     //up
//     6, 3, 2,
//     3, 6, 7,

//     //down
//     5, 1, 0,
//     4, 5, 0
// };

//each integer in this vector represents an index in the VBdata list divided by 3 (vertices).
//every two integers represent vertices that form an edge
std::vector<unsigned int> cubeEdges = 
{
    //front
    0,1,
    1,2,
    2,5,
    5,0,

    //back
    6,7,
    7,8,
    8,9,
    9,6,

    //right
    12,16,
    16,13,
    13,14,
    14,12,

    //left
    18,19,
    19,22,
    22,20,
    20,18

    //top and bottom redundant
};

//unaltered cube vertices in local space. each vertex has 3 floats for the position, 
//2 floats for the texture coordinate, and one for the index of the normal of the face it belongs to
static GLfloat cubeVBDataReal[] = 
{
    //front
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,

    //back
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,

    //right
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 2.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 2.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 2.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 2.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 2.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 2.0f,

    //left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 3.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 3.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 3.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 3.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 3.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 3.0f,

    //up
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 4.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 4.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 4.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 4.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 4.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 4.0f,

    //down
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 5.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 5.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 5.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 5.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 5.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 5.0f
};
//vectors that are normal to one of the faces of the cube
glm::vec3 cubeNormals[] = 
{
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};
std::vector<unsigned int> slantEdges = 
{
    //front
    0,1,
    1,2,
    2,0,

    //back
    3,4,
    4,5,
    5,3,

    //right
    6,10,
    10,7,
    7,8,
    8,6,

    //left
    12,13,
    13,14,
    14,17,
    17,12

    //bottom is redundant
};

static GLfloat slantVBData[] = 
{
    //front
    //bl
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
    //br
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
    //tr
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f,

    //back
    //tr
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
    //br
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    //bl
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,

    //right
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 2.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 2.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 2.0f,

    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 2.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 2.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 2.0f,

    //left
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 3.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 3.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 3.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 3.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 3.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 3.0f,
    //up (n/a)

    //down
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 4.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 4.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 4.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 4.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 4.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 4.0f

};
glm::vec3 slantNormals[] = 
{
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(-0.7071067811865475f, 0.7071067811865475f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};

//globals used when creating the cylinder part.
//we can customize the number of edges the cylinder has (how smooth it looks) and it's radius.
const int numCylinderEdges = 8;
const float cylinderRadius = 0.5f;
std::vector<unsigned int> cylinderEdges; 
static GLfloat cylinderVBData[((numCylinderEdges * 3 * 2) + (numCylinderEdges * 6))*6];
glm::vec3 cylinderNormals[numCylinderEdges + 2] = 
{
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};

//sets a vertex ()
void setVertex(GLfloat* start, glm::vec3 vertCoords, glm::vec2 texCoords, float normInd)
{
    start[0] = vertCoords[0]; start[1] = vertCoords[1]; start[2] = vertCoords[2]; start[3] = texCoords[0]; start[4] = texCoords[1]; start[5] = normInd;
}

//creates the vertices of the cylinder part based on the number of edges on the sides and the radius.
void populateCylinderVBData()
{
    //top, then bottom, then sides
    GLfloat* cy = cylinderVBData;
    float angleIncrement = (2.0f * M_PI)/numCylinderEdges;
    float top = 0.5f;
    for(int i = 0; i < numCylinderEdges; i++)
    {
        int NI = i + 2;

        //now we create normals (except for top and bottom)
        cylinderNormals[NI] = glm::vec3(cosf(angleIncrement * (i + 0.5f)) * cylinderRadius, 0.0f, -sinf(angleIncrement * (i + 0.5f)) * cylinderRadius);
        // cylinderNormals[NI] = glm::vec3(1.0f,0.0f,0.0f);
        float cyX = cosf(angleIncrement * i) * cylinderRadius;
        float cyZ = sinf(angleIncrement * i) * cylinderRadius;
        float cyX2 = cosf(angleIncrement * (i+1)) * cylinderRadius;
        float cyZ2 = sinf(angleIncrement * (i+1)) * cylinderRadius;

        //top
        GLfloat* start = cy + (i * 6 * 3);
        unsigned int index = (start - cy);
        cylinderEdges.push_back(index/6);
        // std::cout << "pushing back " << index  << "|  " << cylinderEdges.size() << '\n';
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f), 0);
        start += 6; index += 6;
        cylinderEdges.push_back(index/6);
        setVertex(start, glm::vec3(cyX2, top, -cyZ2), glm::vec2(0.0f,0.0f), 0);
        start += 6; index += 6;
        setVertex(start, glm::vec3(0.0f, top, 0.0f), glm::vec2(0.0f,0.0f), 0);
        
        

        //bottom
        start = cy + (i * 6 * 3) + (numCylinderEdges * 3 * 6);
        index = (start - cy);
        cylinderEdges.push_back(index/6);
        setVertex(start, glm::vec3(0.0f, -top, 0.0f), glm::vec2(0.0f,0.0f), 1);
        start += 6; index += 6;
        cylinderEdges.push_back(index/6);
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f), 1);
        start += 6; index += 6;
        setVertex(start, glm::vec3(cyX, -top, -cyZ), glm::vec2(0.0f,0.0f), 1);

        //sides
        start = cy + (i * 6 * 6) + (numCylinderEdges * 3 * 6 * 2);
        index = (start - cy);
        cylinderEdges.push_back(index/6);
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f), NI);
        start += 6; index += 6;
        cylinderEdges.push_back(index/6);
        setVertex(start, glm::vec3(cyX, -top, -cyZ), glm::vec2(0.0f,0.0f), NI);
        start += 6; index += 6;
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f), NI);

        start += 6; index += 6;
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f), NI);
        start += 6; index += 6;
        setVertex(start, glm::vec3(cyX2, top, -cyZ2), glm::vec2(0.0f,0.0f), NI);
        start += 6; index += 6;
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f), NI);

        
    }
}


// std::vector<PartData*> partPool(32);
// std::vector<PhysicsData*> physicsPool(32); 
// std::vector<int> partTypes(32);
// vector<int> groupIndexPool(32, -1);
// vector<node> connectedTree(32);
// vector<group> groups;
// std::vector<partInds> indPool(32);

//all groups of objects in the scene
list<group> groups(0);
list<group>::iterator groupsEnd;
//stores information about each part such as it's group, what type it is, and where it is in the vector of parts of it's type.
std::vector<partInfo> pool(32);



int totalParts = 0;
ShaderProgram cubeProgram{cubeVertShaderSource, lightingFragShader};
ShaderProgram outlineProgram{cubeVertShaderSource, outlineFragShaderSource};
static VAOData cubeData{cubeProgram, 0, 0, cubeVBDataReal, sizeof(cubeVBDataReal), cubeNormals, 6, GAMECUBE, cubeEdges, vector<vec3>{}};

static VAOData slantData{cubeProgram, 0, 0, slantVBData, sizeof(slantVBData), slantNormals, 5, GAMESLANT, slantEdges, vector<vec3>{}};

static VAOData cylinderData{cubeProgram, 0, 0, cylinderVBData, sizeof(cylinderVBData), cylinderNormals, numCylinderEdges + 2, GAMECYLINDER, cylinderEdges, vector<vec3>{}};

inline PartData& indsToPart(partInds inds)
{
    return(VAOs[inds.VAOInd]->parts[inds.vecInd]);
}
inline PhysicsData& indsToPhys(partInds inds)
{
    return(VAOs[inds.VAOInd]->partsPhysics[inds.vecInd]);
}

inline PartData& allParts(int poolInd)
{
    // std::cout << "all\n";
    // if(poolInd < 0)
    //     return(indsToPart(pool[0].inds));
    // std::cout << poolInd << " " << pool.size();
    // std::cout << pool[poolInd].inds.VAOInd << pool[poolInd].inds.vecInd << '\n';
    // PartData& hi = indsToPart(pool[poolInd].inds);
    // std::cout << "end\n";
    return(indsToPart(pool[poolInd].inds));
}
inline PhysicsData& allPhys(int poolInd)
{
    // if(poolInd < 0 || poolInd > totalParts)
    //     return(indsToPhys(pool[0].inds));
    // std::cout << poolInd << " " << pool.size();
    return(indsToPhys(pool[poolInd].inds));
}

VAOData* VAOs[] = 
{
    &cubeData, &slantData, &cylinderData
};
int numVAOs = sizeof(VAOs)/sizeof(VAOData*);

//find the linear velocity of a point based off of how fast the center point it is connected to is spinning.
vec3 pointLinearDueToAngular(vec3 pt, vec3 center, vec3 angular)
{
    // if(groupIndexPool[partIndex] == -1)
    //     return(vec3(0.0f,0.0f,0.0f));
    vec3 centerToPt = pt - center;
    vec3 vel = glm::cross(angular, centerToPt);
    return vel;
}

//add a part as a member of a group
void addToGroup(int partIndex, group* groupPtr)
{
    group& currGroup = *groupPtr;
    currGroup.members.push_back(partIndex);
    // int num = currGroup.members.size();
    // PartData& part = *(partPool[partIndex]);
    // PhysicsData& phys = *(physicsPool[partIndex]);
    PartData& part = allParts(partIndex);
    PhysicsData& phys = allPhys(partIndex);
    pool[partIndex].groupPtr = groupPtr;
    phys.linearV = currGroup.linearV;
    phys.angularV = currGroup.angularV;
    currGroup.COM = ((currGroup.COM * currGroup.totalMass) + (part.translate * phys.mass))/(currGroup.totalMass + phys.mass);
    currGroup.totalMass += phys.mass;
    std::cout << currGroup.totalMass << "mass\n";
}


//prepare part to be removed from it's group
void markRemovalFromGroup(int poolIndex, group* groupPtr)
{
    //mark group for fixing (we do this in other function later)
    vector<int>& mems = groupPtr->members;
    mems.erase(std::remove(mems.begin(), mems.end(), poolIndex));
    
    //now remove the part from adjacent lists
    detachPart(poolIndex);

    return;
    //---------------------------------------------------------------------------

    group& currGroup = *groupPtr;
    currGroup.members.erase(std::find(currGroup.members.begin(),currGroup.members.end(), poolIndex));
    pool[poolIndex].groupPtr = NULL;

    PartData& part = allParts(poolIndex);
    PhysicsData& phys = allPhys(poolIndex);
    phys.linearV = currGroup.linearV + pointLinearDueToAngular(part.translate, currGroup.COM, currGroup.angularV);
    phys.angularV = vec3(0.0f,0.0f,0.0f);
    //if group is empty, destroy it
    if(currGroup.members.size() == 0)
    {
        // groupErase(groupPtr);
        return;
    }
    //if there is one member in group, there is no point in having group now
    if(currGroup.members.size() == 1)
    {
        markRemovalFromGroup(currGroup.members[0], groupPtr);
        return;
    }
    currGroup.COM = ((currGroup.COM * currGroup.totalMass) + (part.translate * phys.mass))/(currGroup.totalMass + phys.mass);
    currGroup.totalMass += phys.mass;

}


//for a certain VAO, fills the ptsOnNorms vector with points on the part that each correspond to one of it's normal vectors which corresponds to the face the point is on.
void fillPtsOnNormals(VAOData* VAO)
{
    float* verts = VAO->VBData;
    //for each normal
    for(int i = 0; i < VAO->numNormals; i++)
    {
        glm::vec3 currNorm = VAO->normalsArray[i];
        float maxProj = INT_MIN;
        glm::vec3 maxPt;
        //for each point on the part, find if it is the furthest in the direction of the current normal
        for(int j = 0; j < (VAO->VBDataSize)/(sizeof(float)*6); j++)
        {
            glm::vec3 currPt = *((glm::vec3*)((verts) + (j*6)));
            float currProj = glm::dot(currNorm, currPt);
            if(maxProj < currProj)
            {
                maxProj = currProj;
                maxPt = currPt;
            }
        }
        VAO->ptsOnNorms.push_back(maxPt);
    }
}

//create the shader programs and send the VBData for each part type to the gpu
void initializeBuiltInParts()
{
    std::cout << "START INIT \n";
    populateCylinderVBData();
    cubeProgram.createProgram();
    outlineProgram.createProgram();
    // std::cout << "cyledge size " << VAOs[GAMECYLINDER]->partEdges.size() << '\n';
    int currOffset = 0;
    glm::vec3 allNorms[21];
    for(int i = 0; i < sizeof(VAOs)/sizeof(VAOData*); i++)
    {
        generateVAO(VAOs[i], currOffset, allNorms);
        fillPtsOnNormals(VAOs[i]);
    }
    //we set the uniform representing all the norms in the shader program
    vec3 lp(-4.0f, 4.0f, 0.0f);
    glUseProgram(cubeProgram.programID);
    glUniform3fv(glGetUniformLocation(cubeProgram.programID, "norms"), 21, (GLfloat*)allNorms);
    glUniform3f(glGetUniformLocation(cubeProgram.programID, "lightPos"), lp.x,lp.y,lp.z);
    std::cout << VAOs[0]->VBOID << " and then " << VAOs[1]->VBOID <<'\n';
}

//create the VAO and VBO for every part type
void generateVAO(VAOData* data, int& currOffset, glm::vec3* allNorms)
{
    data->shader = cubeProgram;
    data->offset = currOffset;
    for(int i = 0; i < data->numNormals; i++)
    {
        allNorms[i + currOffset] = data->normalsArray[i];
    }
    currOffset += data->numNormals;
    // data->shader.createProgram();
    //create VAO and buffers for vertex coords
    glGenVertexArrays(1, &(data->VAOID));
    glBindVertexArray(data->VAOID);
    
    glGenBuffers(1, &(data->VBOID));
    glBindBuffer(GL_ARRAY_BUFFER, data->VBOID);
    glBufferData(GL_ARRAY_BUFFER, (data->VBDataSize), (data->VBData), GL_STATIC_DRAW);
    
    //attribute for local positions of vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    //attribute for texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //attrib for normal index
    glVertexAttribPointer(13, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(13);

    //create buffers and vertex attributes for PartData

    glGenBuffers(1, &(data->partsBOID));
    glBindBuffer(GL_ARRAY_BUFFER, data->partsBOID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PartData) * data->parts.capacity(), (void*)(data->parts.data()), GL_DYNAMIC_DRAW);

    //attribute for rotation matrix (mat4)
    for(int i = 2; i <= 5; i++)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(float) * (i-2) * 4));
        glVertexAttribDivisor(i, 1);
    }
    //attribute for translate vector
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4)));
    glVertexAttribDivisor(6, 1);

    //attrib for color vector
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4) + sizeof(glm::vec3)));
    glVertexAttribDivisor(7, 1);

    //attrib for scale vector
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(glm::vec3)));
    glVertexAttribDivisor(8, 1);

    //attribute for normal matrix (mat3)
    for(int i = 9; i <= 11; i++)
    {
        size_t offset = (sizeof(float) * (i-9) * 3);
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + offset));
        glVertexAttribDivisor(i, 1);
    }

    //attrib for texture unit
    glEnableVertexAttribArray(12);
    glVertexAttribPointer(12, 1, GL_INT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::mat3)));
    glVertexAttribDivisor(12, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


//adds part with default state.
PartData* VAOData::addPart()
{
    if(parts.size() == numParts)
    {
        parts.resize(numParts * 2);
        partsPhysics.resize(numParts * 2);
        glBindBuffer(GL_ARRAY_BUFFER, partsBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(PartData) * parts.capacity(), (void*)(parts.data()), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if(pool.size() == totalParts)
    {
        pool.resize(totalParts * 2);
    }
    pool[totalParts] = partInfo{partType, NULL, node{}, partInds{partType, numParts}};
    numParts++;
    totalParts++;
    return(&parts[numParts - 1]);
}
//render all the parts of the current part type
void VAOData::drawParts(bool selected, bool outline, bool drawTransformationParts)
{
    if(selected && drawTransformationParts)
    {
        std::cout << "CANNOT DRAW transformation parts and selected parts at the same time\n";
        return;
    }
    //set the shader program and uniforms
    GLuint currShader = shader.programID;
    if(selected && outline)
        currShader = outlineProgram.programID;
    glUseProgram(currShader);
    glUniformMatrix4fv(glGetUniformLocation(currShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(currShader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(glGetUniformLocation(currShader, "offset"), offset);
    //part outlines are not affected by lighting
    if(!selected)
    {
        glUniform3f(glGetUniformLocation(cubeProgram.programID, "camPos"), camPos.x,camPos.y,camPos.z);
        glUniform3f(glGetUniformLocation(cubeProgram.programID, "lightPos"), lightPos.x,lightPos.y,lightPos.z);
    }

    glBindVertexArray(VAOID);

    //draw all parts except scale parts
    if(!selected && !drawTransformationParts)
    {
        int first = (partType ==  GAMECUBE ? 6 : 0);
        // glDrawArraysInstanced(GL_TRIANGLES, 0, VBDataSize/(sizeof(GLfloat)*5), numParts);
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VBDataSize/(sizeof(GLfloat)*6), numParts - first, first);
    }
    //draw part outlines for selected parts
    else if(selected)
    {
        // return;
        for(int i = 0; i < selectedParts.size(); i++)
        {
            partInfo part = pool[selectedParts[i]];
            int partVecIndex = part.inds.vecInd;
            if(part.type != partType)
                continue;
            glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VBDataSize/(sizeof(GLfloat)*6), 1, partVecIndex);
        }
    }
    //draw scale parts
    else if(drawTransformationParts && partType == GAMECUBE)
    {
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VBDataSize/(sizeof(GLfloat)*6), 6, 0);
    }
    glBindVertexArray(0);
}

//If all is true, updates the entire buffer. Otherwise, updates the buffer from start to end index (inclusive)
void VAOData::updateParts(bool all, int start, int end)
{   glBindBuffer(GL_ARRAY_BUFFER, partsBOID);

    if(all)
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PartData) * parts.capacity(), (void*)(parts.data()));
    else
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(PartData) * start, sizeof(PartData) * (end - start + 1), (void*)(&parts[start]));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//checking if every member of two groups are equal
bool group::operator==(const group& g2)
{
    return(
        totalMass == g2.totalMass
        && COM == g2.COM
        && linearV == g2.linearV
        && angularV == g2.angularV
        && members == g2.members
        && index == g2.index
        );
}

void markAllUnvisited()
{
    for(int i = 0; i < pool.size(); i++)
        pool[i].nodeInfo.visited = false;
}

//returns true if there is a path between two nodes (parts) or a node and a group 
//-in the connectedParts tree.
// bool pathBetweenNodes(int index1, int index2, bool ind2Group)
// {
//     std::vector<int>* mems;
//     if(ind2Group)
//         mems = &(pool[index2].groupPtr->members);
//     else
//         std::cout << "HELP ME\n";
//     markAllUnvisited();
//     //bfs
//     std::deque<int> queue{index1};
//     while(queue.size() > 0)
//     {
//         int currInd = queue[0];
//         queue.pop_front();
//         //if the current part is what we are looking for, return true
//         if(!ind2Group && currInd == index2)
//             return true;
//         //if the current part is in the group we are looking for, return true
//         else if(ind2Group && std::find(mems->begin(), mems->end(), currInd) != mems->end())
//             return true;
//         vector<int>& adjNodes = pool[currInd].nodeInfo.adjacent;
//         for(const int& adj : adjNodes)
//         {
//             if(pool[adj].nodeInfo.visited == false)
//             {
//                 queue.push_back(adj);
//                 pool[adj].nodeInfo.visited = true;
//             }
//         }
//     }
//     return false;
// }