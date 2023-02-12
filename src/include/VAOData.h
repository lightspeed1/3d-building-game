#pragma once
#define GLEW_STATIC 1
#include "glew.h"
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"
#include <vector>
#include <list>
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

    unsigned short partType;
    
    std::vector<unsigned int>& partEdges;
    std::vector<glm::vec3> ptsOnNorms;
    //variables below this line are initialized at runtime by initializeBuiltInParts()
    int offset;
    unsigned int partsBOID;
    unsigned short numParts{0};
    std::vector<PartData> parts{16};
    std::vector<PhysicsData> partsPhysics{16};
    PartData* addPart();
    void drawParts(bool selected, bool outline, bool drawTransformationParts);
    void updateParts(bool all, int start, int end);
};
#include <list>

struct node
{
    bool visited = false;
    std::vector<int> adjacent;
};

struct group
{
    float totalMass = 0.0f;
    glm::vec3 COM;
    glm::vec3 linearV;
    glm::vec3 angularV;
    int index;
    std::vector<int> members;
    bool operator==(const group& g2);
};

struct partInds
{
    unsigned short VAOInd;
    unsigned short vecInd;
};

struct partInfo
{
    int type;
    group* groupPtr;
    node nodeInfo;
    partInds inds;
    int newIndex = -1;
    int newPhysIndex = -1;
};
// extern std::vector<int> partTypes;
// extern std::vector<PartData*> partPool;
// extern std::vector<PhysicsData*> physicsPool;
// extern std::vector<int> groupIndexPool;
// extern std::vector<node> connectedTree;
extern std::vector<partInfo> pool;
//we use a list because the groups don't need to be contiguous unlike partData
//-and pointers to list elements are always valid unless we remove said element 
extern std::list<group> groups;
extern std::list<group>::iterator groupsEnd;
// extern std::vector<group> groups;
extern int totalParts;
// extern std::vector<partInds> indPool;


extern inline PartData& indsToPart(partInds inds);
extern inline PhysicsData& indsToPhys(partInds inds);
extern inline PartData& allParts(int poolInd);
extern inline PhysicsData& allPhys(int poolInd);
extern void markRemovalFromGroup(int poolIndex, group* groupPtr);
extern void addToGroup(int partIndex, group* groupPtr);
extern void markAllUnvisited();
extern glm::vec3 pointLinearDueToAngular(glm::vec3 pt, glm::vec3 center, glm::vec3 angular);
extern VAOData* VAOs[];
extern int numVAOs;

extern void initializeBuiltInParts();
extern void generateVAO(VAOData* data, int& currOffset, glm::vec3* allNorms);