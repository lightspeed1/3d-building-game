#pragma once
#include <iostream>
#include <vector>
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#include "../glm/glm/gtx/intersect.hpp"
#include "include/VAOData.h"
#include "include/PhysicsData.h"
#include "include/DrawPrims.h"
#include "include/game.h"
#include "include/PartData.h"
#include <algorithm>

extern int currNewPart;
extern std::vector<int> selectedParts;
extern std::vector<group*> groupsToFix;
extern void spawnPart(glm::vec3 pos);
extern void setGroupMassAndCOM(group* g);
extern void combineParts(std::vector<int>& p, int draggedPart);
extern void copyParts();
extern void deleteParts();
extern void fixGroups();
extern void addPart();
extern void scaleDrag(int partIndex, int scalePartInd, glm::vec3& dragPt);