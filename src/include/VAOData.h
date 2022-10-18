#pragma once
#define GLEW_STATIC 1
#include "glew.h"
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"
#include <vector>
#include "PartData.h"
#include "ShaderProgram.h"
#include "PhysicsData.h"
#define GAMECUBE 0
#define GAMESLANT 1
#define GAMECYLINDER 2
struct VAOData
{
    ShaderProgram shader;
    unsigned int VAOID;
    unsigned int VBOID;
    // unsigned int EBOID;

    GLfloat* VBData;
    // GLuint* EBData;
    unsigned int VBDataSize;
    // unsigned int EBDataSize;
    
    glm::vec3* normalsArray;
    int numNormals;

    int partType;
    
    //variables below this line are initialized at runtime by initializeBuiltInParts()
    unsigned int partsBOID;
    unsigned int numParts{0};
    std::vector<PartData> parts{16};
    std::vector<PhysicsData> partsPhysics{16};
    PartData* addPart();
    void drawParts();
    void updateParts(bool all, int start, int end);
};

extern std::vector<int> partTypes;
extern std::vector<PartData*> partPool;
extern std::vector<PhysicsData*> physicsPool; 
extern int totalParts;

extern VAOData* VAOs[];
extern int numVAOs;

void initializeBuiltInParts();
void generateVAO(VAOData* data);