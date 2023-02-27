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
extern std::vector<PartData> partsToCopy;
extern glm::vec3 currNewColor;
extern std::vector<group*> groupsToFix;
extern void spawnPart(glm::vec3 pos);
extern void setGroupMassAndCOM(group* g);
extern void combineParts(std::vector<int>& p, int draggedPart, bool multipleDrag);
extern glm::vec3 maxPtInDirMult(std::vector<int> partIndices, glm::vec3 dir);
extern void copyParts();
extern void pasteParts();
extern glm::vec3 centerOfParts(std::vector<int>& partIndices);
extern void deleteParts();
extern void fixGroups();
extern void addPart();
extern void scalePartDrag(std::vector<int> partIndices, int scalePartInd, glm::vec3& dragPt, bool scale);