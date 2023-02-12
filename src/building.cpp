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
#include "include/building.h"
#include <algorithm>
#include <deque>
using namespace std;
using namespace glm;


//type of part to add if user wants to add a new part
int currNewPart = GAMECUBE;

//indices of selected parts in the partPool
vector<int> selectedParts;

vector<group*> groupsToFix;


//finds the distance from a point to a line (closest distance from the point to any point on the line)
glm::vec3 lineToPoint(glm::vec3 line, glm::vec3 pt)
{
    line = glm::normalize(line);
    vec3 rawDist = pt - line;
    vec3 realDist = rawDist - (line * glm::dot(line, rawDist));
    return(realDist);
}

//this function changes the scale of a part based on how much far of the scaling parts has been dragged.
void scaleDrag(int partIndex, int scalePartInd, glm::vec3& dragPt)
{
    //shoot out a ray that represents where the user is clicking
    glm::vec3 ray = shootRay();
    vec3 origDir = VAOs[GAMECUBE]->normalsArray[scalePartInd];
    PartData& part = allParts(partIndex);
    PartData& scalePart = allParts(scalePartInd);
    glm::vec3 dir = part.applyNM(origDir);
    glm::vec3 pointToLine = -lineToPoint(ray, dragPt - camPos);
    vec3 absValueOrig = vec3(abs(origDir.x),abs(origDir.y), abs(origDir.z));
    float increase = glm::dot(dir, pointToLine);

    //change the scale for every 1 meter the user drags the scale part in the direction the scale part represents.
    if(fGEqual(abs(increase/0.5),1))
    {
        vec3 ptOnPart = vec3(glm::transpose(part.rotationMatrix) * vec4(scalePart.translate - dir - part.translate, 1.0f));
        ptOnPart = vec3(part.rotationMatrix * vec4(ptOnPart/part.scaleVector, 1.0f));
        increase = (int)(increase/0.5) * 0.5;
        float scaleAdd = increase/glm::dot(dir,ptOnPart);
        vec3 newScale = (scaleAdd * absValueOrig) + part.scaleVector;
        if(v3Equal(newScale, glm::vec3(0.0f,0.0f,0.0f)))
            return;
        part.scale(newScale);
        part.translate = part.translate + (dir * increase);
        dragPt = dragPt + (dir * increase * 2.0f);
    }
}


//moves the topIndex part to be right above the bottomIndex part.
void movePartAbove(int topIndex, int bottomIndex)
{
    int i = topIndex;
    int j = bottomIndex;
    PartData& part1 = allParts(i);
    PartData& part2 = allParts(j);

    vec3 upVec = glm::vec3(0.0f,1.0f,0.0f);
    vec3 maxPtUpJ = maxPtInDir(j, upVec);
    vec3 posAdd1 = upVec * (glm::dot(upVec, maxPtUpJ));
    vec3 maxPtDownI = maxPtInDir(i, -upVec);
    vec3 posAdd2 = upVec * glm::dot(upVec, part1.translate - maxPtDownI);
    //lets move posAdd2 down a tad bit so it collides with the part below it
    //SUTRACTING 0.001 so they collide
    posAdd2 = glm::normalize(posAdd2) * (glm::length(posAdd2) - 0.001f); 
    vec3 verticalFinal = posAdd1 + posAdd2;
    vec3 horizFinal = part1.translate - (upVec * glm::dot(upVec, part1.translate));
    vec3 finalPos = verticalFinal + horizFinal;
    part1.translate = finalPos;
}

//changes the position an object that was just added by moving it above any other objects it is colliding with.
void adjustNewObjPos(int topIndex, int bottomIndex, vector<int>& touching)
{
    allParts(topIndex).translate = allParts(bottomIndex).translate;
    movePartAbove(topIndex, bottomIndex);
    
    bool negative = false;
    bool doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching);
    while(!doneAdjusting)
    {
        negative = false;
        for(int k = 0; k < touching.size(); k++)
        {
            int currIndex = touching[k];
            if(currIndex < 0)
            {
                bottomIndex = abs(currIndex);
                movePartAbove(topIndex, abs(currIndex));
                negative = true;
                touching.clear();
                break;
            }
        }
        if(!negative)
            std::cout << "NO NEGATIVES??\n";
        else
            std::cout << "yes negatives\n";
        //now that we may have changed the position of the top part, we must check again if it is colliding with any other parts.
        doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching);
    }
}

//add the a part of the current new type to the workspace.
void addPart()
{
    PartData& newPart = *(VAOs[currNewPart]->addPart());
    newPart.color = vec3(1.0f,0.0f,0.0f);
    int bottomPartInd = 0;
    if(selectedParts.size() > 0)
        bottomPartInd = selectedParts[0];

    //we set the position of this part to above the currently selected part.
    //if no parts are selected, set the position to above the ground part.
    //if there are parts above this bottom part, the highest part is the new bottom part (our new part is built on top of it)

    //we will use the partsrayintersects function to find the real bottom part.
    vec3 botVec = glm::vec3(0.0f, -1.0f, 0.0f);
    vec3 rayOrigin = allParts(bottomPartInd).translate + glm::vec3(0.0f,100.0f, 0.0f);
    vec3 intPt;
    int intNormIndex;
    int trueBottomPart = partsRayIntersects(true, botVec, rayOrigin, &intPt, &intNormIndex, false);
    std::cout << "TRUE BOTTOM PART IS " << trueBottomPart << '\n';
    vector<int> touching;
    int topPart = totalParts-1;
    adjustNewObjPos(topPart, trueBottomPart, touching);
    touching.push_back(topPart);

    for(int i = 0; i < touching.size(); i++)
        if(allPhys(touching[i]).stationary == true)
            removeFromVec(touching, touching[i]);
    
    //now we add the objects that are touching to a group together.
    if(touching.size() > 1)
        combineParts(touching, topPart);
}

//calculates the total mass and center of mass for a group which consists of 1 or more parts.
void setGroupMassAndCOM(group* g)
{
    vec3 COM = vec3(0.0f,0.0f,0.0f);
    float totalMass = 0.0f;
    vector<int>& mems = g->members;
    for(int i = 0; i < mems.size(); i++)
    {
        float currMass = allPhys(mems[i]).mass; vec3 currPos = allParts(mems[i]).translate;
        totalMass += currMass;
        COM += currMass * currPos;
    }
    if(!v3Equal(COM, vec3(0.0f,0.0f,0.0f)))
        COM = COM/totalMass;
    g->COM = COM; g->totalMass = totalMass;
}

void fixGroups()
{
    //for every group we need to fix, we do bfs on it one or more times until we have visited all members.
    //each time we do bfs, the nodes we visit will be their own group.
    //we create a new group from these parts
    for(int i = 0; i < groupsToFix.size(); i++)
    {
        group& currOld = *(groupsToFix[i]);
        vector<int>& oldMems = currOld.members;
        deque<int> toVisit;

        while(oldMems.size() > 0)
        {
            groups.push_back(group());
            group* currNew = &(groups.back());

            toVisit.push_back(oldMems.back());
            oldMems.pop_back();
            while(toVisit.size() > 0)
            {
                //add node to new members and pop from to visit
                int currNode = toVisit.front();
                currNew->members.push_back(currNode);
                toVisit.pop_front();
                pool[currNode].groupPtr = currNew;
                vector<int>& adj = pool[currNode].nodeInfo.adjacent;
                //get all adjacent nodes to current node and visit them soon
                for(int j = 0; j < adj.size(); j++)
                {
                    if(std::find(oldMems.begin(),oldMems.end(), adj[j]) != oldMems.end())
                    {
                        toVisit.push_back(adj[j]);
                        oldMems.erase(std::remove(oldMems.begin(),oldMems.end(),adj[j]));
                    }
                }
            }
            
            // //if new group is all connected, don't do anything
            // if(currNew->members.size() == oldMems.size())
            // {
            //     killGroup(&currOld, false);
            //     return;
            // }

            //if the current group consists of one part, there is no point in having a group (destroy group)
            glm::vec3* newAngVel, *newLinVel, *newCenter;
            if(currNew->members.size() == 1)
            {
                // std::cout << "A\n";
                int lonePart = currNew->members[0];
                newAngVel = &(allPhys(lonePart).angularV);
                newLinVel = &(allPhys(lonePart).linearV);
                newCenter = &(allParts(lonePart).translate);
                std::cout << newCenter->x << '\n';
                killGroup(currNew, true);
            }
            else
            {
                // std::cout << "B\n";
                setGroupMassAndCOM(currNew);
                newAngVel = &(currNew->angularV);
                newLinVel = &(currNew->linearV);
                newCenter = &(currNew->COM);
            }
            //now we find lin and angular vel of new group
            vec3 newLinV = currOld.linearV + pointLinearDueToAngular(*newCenter, currOld.COM, currOld.angularV);
            vec3 newAngV = currOld.angularV;
            *newAngVel = newAngV; *newLinVel = newLinV;
        }
        killGroup(&currOld, false);
    }
    if(groupsToFix.size() == 0)
        return;
    groupsToFix.clear();
    // std::cout << "IN FIXGROUPS GROUPS ARE:\n";
    // for(const auto& currGroup: groups)
    // {
    //     for(int i = 0; i < currGroup.members.size(); i++)
    //     {
    //         std::cout << currGroup.members[i] << '\n';
    //     }
    //     std::cout << "next\n";
    // }
}

void spawnPart(vec3 pos)
{
    PartData* newPart = VAOs[currNewPart]->addPart();
    newPart->translate = pos;
}

//copies all selected parts. currently not used
void copyParts()
{
    vector<int> newSelection;
    for(int i = 0; i < selectedParts.size(); i++)
    {
        newSelection.push_back(totalParts);
        int j = selectedParts[i];
        PartData* newPart = VAOs[pool[j].type]->addPart();
        (*newPart) = allParts(j);
    }
    selectedParts = std::move(newSelection);
}

//combines parts into a group. if the parts are already in
//their own groups, combines the groups
//also, add each part to each others adjacency lists
void combineParts(std::vector<int>& p, int draggedPart)
{
    groups.push_back(group());
    group& newGroup = groups.back(); 
    vector<int>& nMems = newGroup.members;
    for(int i = 0; i < p.size(); i++)
    {
        group* currGroup = pool[p[i]].groupPtr;
        if(currGroup != NULL)
        {
            // std::cout << "has group" << currGroup->members.size() << '\n';
            nMems.insert(nMems.end(), currGroup->members.begin(), currGroup->members.end());
            killGroup(currGroup, true);
        }
        else
        {
            nMems.insert(nMems.end(), p[i]);
        }
        // pool[p[i]].groupPtr = &newGroup;
    }
    for(int j = 0; j < nMems.size(); j++)
            pool[nMems[j]].groupPtr = &newGroup;
    std::sort(nMems.begin(), nMems.end());
    nMems.erase(std::unique(nMems.begin(),nMems.end()),nMems.end());
    //now we have removed duplicates from the members, we can find the COM and totalMass
    //we could also calculate the velocity of new group, but no parts will be combined when parts are moving.
    setGroupMassAndCOM(&newGroup);


    //setting the adjacency lists of the parts
    for(int i = 0; i < p.size(); i++)
    {
        vector<int>& adj = pool[p[i]].nodeInfo.adjacent; 
        if(p[i] == draggedPart)
        {
            adj = p;
            adj.erase(std::remove(adj.begin(), adj.end(), p[i]));
        }
        else
        {
            if(std::find(adj.begin(), adj.end(), draggedPart) == adj.end())
                adj.push_back(draggedPart);
        }
    }
    // std::cout << "GROUP IN COMBINE PARTS IS \n";
    // for(int i = 0; i < newGroup.members.size(); i++)
    // {
    //     std::cout << newGroup.members[i] << '\n';
    // }
    
}

bool shouldRemovePool(partInfo& p) {return(p.newIndex == -2);}
bool shouldRemovePart(PartData& p) {return(p.texUnit == -2);}
bool shouldRemovePhys(PhysicsData& p) {return(p.mass == -2);}

//completely deletes selected parts and all traces of them
void deleteParts()
{
    std::cout << "STARTIT\n";
    //remove from adjacent
    //remove from groups
    //mark for death, while subbing indices
    if(selectedParts.size() == 0)
        return;
    int offset = 0;
    std::sort(selectedParts.begin(), selectedParts.end());
    int selectedIndex = 0;
    vector<int> partDataIndices(numVAOs, 0);
    vector<int> partDataOffsets(numVAOs, 0);
    std::cout << "START MARKING\n";
    for(int i = 0; i < totalParts; i++)
    {
        int currVAO;
        //if curr part is one to be deleted
        if(selectedIndex < selectedParts.size() && selectedParts[selectedIndex] == i)
        {
            //prevent the base part from being deleted
            if(selectedParts[selectedIndex] == 6)
            {
                return;
            }
            currVAO = pool[i].inds.VAOInd;
            offset--;
            partDataOffsets[currVAO]--;
            VAOs[currVAO]->numParts -= 1;
            //mark pool element and partData and physicsData elements for death
            pool[i].newIndex = -2;
            allParts(i).texUnit = -2;
            allPhys(i).mass = -2;
            //remove from adjacent
            removeFromAdjacent(i);
            //remove from group
            group* groupPtr = pool[i].groupPtr;
            if(groupPtr != NULL)
            {
                markRemovalFromGroup(i, groupPtr);
            }
            selectedIndex++;
        }

        //WE WILL GO OVER TOTAL PARTS A SECOND TIME AND APPLY THE OFFSETS AFTER THE GROUPS AND ADJ
        //ARRAYS AND MARKS FOR DEATH ARE TAKEN CARE OF


        //ELSE: NOTE DOWN THE NEW INDEX, IT WILL TAKE EFFECT IN A DIFFERENT FOR LOOP OUTSIDE
        //THIS SCOPE.
        else
        {
            currVAO = pool[i].inds.VAOInd;
            int newIndex = i + offset;
            pool[i].newIndex = newIndex;
            pool[i].newPhysIndex = partDataIndices[currVAO] + partDataOffsets[currVAO];
        }
        partDataIndices[currVAO]++;
    }
    std::cout << "HERE\n";
    //NOW REMOVE PARTS -------------------------------------------------------------------------
    //erase things in pool
    //erase things in every VAO
    for(int i = 0; i < numVAOs; i++)
    {
        vector<PartData>& currParts = VAOs[i]->parts;
        vector<PhysicsData>& currPhys = VAOs[i]->partsPhysics;
        currParts.erase(std::remove_if(currParts.begin(), currParts.end(), shouldRemovePart), currParts.end());
        currPhys.erase(std::remove_if(currPhys.begin(), currPhys.end(), shouldRemovePhys), currPhys.end());
        int a = 1;
    }
    //NOW UPDATE PART INDICES--------------------------------------------------------------------
    for(int i = 0; i < totalParts; i++)
    {
        if(std::find(selectedParts.begin(), selectedParts.end(), i) != selectedParts.end())
            continue;
        //we must update curr pool index in its group, and among its adjacent parts
        if(pool[i].groupPtr != NULL)
        {
            // std::cout << "i is \n";
            vector<int>& mems = pool[i].groupPtr->members;
            // std::cout << "mems before change:\n";
            // for(int a : mems) std::cout << a << '\n';
            std::replace(mems.begin(), mems.end(), i, pool[i].newIndex); 
            // std::cout << "mems after change:\n";
            // for(int a : mems) std::cout << a << '\n';
        }
        vector<int>& adj = pool[i].nodeInfo.adjacent;
        for(int j = 0; j < adj.size(); j++)
        {
            int correctInd = pool[adj[j]].newIndex;
            adj[j] = correctInd;
        }
        pool[i].inds.vecInd = pool[i].newPhysIndex;
    }
    pool.erase(std::remove_if(pool.begin(), pool.end(), shouldRemovePool), pool.end());
    totalParts -= selectedParts.size();
    // fixGroups();
    selectedParts.clear();
    scalePartsDisappear();
    std::cout << "DONE WITH IT\n";
}