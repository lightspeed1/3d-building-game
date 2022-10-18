#include "include/VAOData.h"
#include <iostream>
#include "include/ShaderSources.h"
#include "include/game.h"
# define M_PI 3.14159265358979323846


static GLfloat cubeVBData[] = {
    //face1
    //bottom left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    //br
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    //tr
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    //tl
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    //face2
    //bottom left
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    //br
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    //tr
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    //tl
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
};

static GLuint cubeEBData[] = {
    //front
    0, 1, 2,
    0, 2, 3,
    //back
    6, 5, 4,
    7, 6, 4,

    //right
    5, 2, 1,
    5, 6, 2,

    //left
    0, 3, 4,
    3, 7, 4,

    //up
    6, 3, 2,
    3, 6, 7,

    //down
    5, 1, 0,
    4, 5, 0
};
static GLfloat cubeVBDataReal[] = 
{
    //front
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,

    //back
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    //right
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,

    //left
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    //up
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

    //down
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
};
glm::vec3 cubeNormals[] = 
{
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};
static GLfloat slantVBData[] = 
{
    //front
    //bl
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    //br
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    //tr
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,

    //back
    //tr
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    //br
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    //bl
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,

    //right
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,

    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

    //left
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    //up (n/a)

    //down
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,

};
glm::vec3 slantNormals[] = 
{
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};
const int numCylinderEdges = 8;
const int cylinderRadius = 1;
static GLfloat cylinderVBData[((numCylinderEdges * 3 * 2) + (numCylinderEdges * 6))*5];
glm::vec3 cylinderNormals[numCylinderEdges + 2] = 
{
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};

void setVertex(GLfloat* start, glm::vec3 vertCoords, glm::vec2 texCoords)
{
    start[0] = vertCoords[0]; start[1] = vertCoords[1]; start[2] = vertCoords[2]; start[3] = texCoords[0]; start[4] = texCoords[1];
}

void populateCylinderVBData()
{
    //top, then bottom, then sides
    GLfloat* cy = cylinderVBData;
    float angleIncrement = (2.0f * M_PI)/numCylinderEdges;
    float top = 0.5f;
    for(int i = 0; i < numCylinderEdges; i++)
    {
        float cyX = cosf(angleIncrement * i) * cylinderRadius;
        float cyZ = sinf(angleIncrement * i) * cylinderRadius;
        float cyX2 = cosf(angleIncrement * (i+1)) * cylinderRadius;
        float cyZ2 = sinf(angleIncrement * (i+1)) * cylinderRadius;

        //top
        GLfloat* start = cy + (i * 5 * 3);
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX2, top, -cyZ2), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(0.0f, top, 0.0f), glm::vec2(0.0f,0.0f));

        //bottom
        start = cy + (i * 5 * 3) + (numCylinderEdges * 3 * 5);
        setVertex(start, glm::vec3(0.0f, -top, 0.0f), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX, -top, -cyZ), glm::vec2(0.0f,0.0f));

        //sides
        start = cy + (i * 5 * 6) + (numCylinderEdges * 3 * 5 * 2);
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX, -top, -cyZ), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f));

        start += 5;
        setVertex(start, glm::vec3(cyX2, -top, -cyZ2), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX2, top, -cyZ2), glm::vec2(0.0f,0.0f));
        start += 5;
        setVertex(start, glm::vec3(cyX, top, -cyZ), glm::vec2(0.0f,0.0f));

        //now we create normals (except for top and bottom)
        cylinderNormals[2 + i] = glm::vec3(cosf(angleIncrement * (i + 0.5f)) * cylinderRadius, 0.0f, -sinf(angleIncrement * (i + 0.5f)) * cylinderRadius);
    }
}
std::vector<PartData*> partPool{32};
std::vector<PhysicsData*> physicsPool{32}; 
std::vector<int> partTypes{32};
int totalParts = 0;
ShaderProgram cubeProgram{cubeVertShaderSource, cubeFragShaderSource};

static VAOData cubeData{cubeProgram, 0, 0, cubeVBDataReal, sizeof(cubeVBDataReal), cubeNormals, 6, GAMECUBE};

static VAOData slantData{cubeProgram, 0, 0, slantVBData, sizeof(slantVBData), slantNormals, 5, GAMESLANT};

static VAOData cylinderData{cubeProgram, 0, 0, cylinderVBData, sizeof(cylinderVBData), cylinderNormals, numCylinderEdges + 2, GAMECYLINDER};


VAOData* VAOs[] = 
{
    &cubeData, &slantData, &cylinderData
};
int numVAOs = sizeof(VAOs)/sizeof(VAOData*);

void initializeBuiltInParts()
{
    // for(int k = 0; k < 36; k++)
    // {
    //     GLfloat* vS = (cubeEBData[k]*5) + cubeVBData;
    //     glm::vec3 vert{*vS, *(vS + 1), *(vS+2)};
    //     GLfloat* nP = (k*5) + cubeVBDataReal;
    //     glm::vec3 newVert{*nP, *(nP + 1), *(nP+2)};
    //     if(newVert == vert)
    //     {
    //         std::cout << k << '\n';
    //     }
    // }
    populateCylinderVBData();
    for(int i = 0; i < sizeof(VAOs)/sizeof(VAOData*); i++)
    {
        generateVAO(VAOs[i]);
    }
    std::cout << VAOs[0]->VBOID << " and then " << VAOs[1]->VBOID <<'\n';
}
void generateVAO(VAOData* data)
{
    data->shader.createProgram();
    //create VAO and buffers for vertex coords
    glGenVertexArrays(1, &(data->VAOID));
    glBindVertexArray(data->VAOID);
    
    glGenBuffers(1, &(data->VBOID));
    glBindBuffer(GL_ARRAY_BUFFER, data->VBOID);
    glBufferData(GL_ARRAY_BUFFER, (data->VBDataSize), (data->VBData), GL_STATIC_DRAW);
    
    // glGenBuffers(1, &(data->EBOID));
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->EBOID);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, (data->EBDataSize), (data->EBData), GL_STATIC_DRAW);

    //attribute for local positions of vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    //attribute for texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


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
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(PartData), (void*)(sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3)));
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
    // std::cout << "adding part " << numParts << '\n';
    if(parts.capacity() == numParts)
    {
        parts.resize(numParts * 2);
        partsPhysics.resize(numParts * 2);
        glBindBuffer(GL_ARRAY_BUFFER, partsBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(PartData) * parts.capacity(), (void*)(parts.data()), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if(partPool.capacity() == numParts)
    {
        partPool.resize(numParts * 2);
        physicsPool.resize(numParts * 2);
        partTypes.resize(numParts * 2);
        partPool[numParts] = &parts[numParts];
        physicsPool[numParts] = &partsPhysics[numParts];
        partTypes[numParts] = partType;
    }
    numParts++;
    totalParts++;
    return(&parts[numParts - 1]);
}
void VAOData::drawParts()
{
    glUseProgram(shader.programID);
    glUniform3fv(glGetUniformLocation(shader.programID, "cubeNormals"), numNormals, (GLfloat*)normalsArray);
    glUniformMatrix4fv(glGetUniformLocation(shader.programID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.programID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glBindVertexArray(VAOID);
    
    // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, numParts);
    glDrawArraysInstanced(GL_TRIANGLES, 0, VBDataSize/sizeof(GLfloat), numParts);
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