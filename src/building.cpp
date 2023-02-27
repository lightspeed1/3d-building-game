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
vec3 currNewColor(1.0f,0.0f,0.0f);

//indices of selected parts in the partPool
vector<int> selectedParts;

//parts stored in clipboard
vector<PartData> partsToCopy;
vector<int> partsToCopyTypes;
vector<group*> groupsToFix;


//finds the distance from a point to a line (closest distance from the point to any point on the line)
glm::vec3 lineToPoint(glm::vec3 line, glm::vec3 pt)
{
    line = glm::normalize(line);
    vec3 rawDist = pt - line;
    vec3 realDist = rawDist - (line * glm::dot(line, rawDist));
    return(realDist);
}

//this function changes the scale OR POSITION of a part based on how much far of the scaling parts has been dragged.
//the scale boolean controls whether the part is moved or scaled up/down.
void scalePartDrag(std::vector<int> partIndices, int currScalePartInd, glm::vec3& dragPt, bool scale)
{
    // if scale == false (we must move) then move all selected parts.
    // also check if parts are over colliding with others. If so, don't move

    //shoot out a ray that represents where the user is clicking
    glm::vec3 ray = shootRay();
    vec3 origDir = VAOs[GAMECUBE]->normalsArray[currScalePartInd];
    
    PartData& scalePart = allParts(currScalePartInd);

    glm::vec3 dir = origDir;
    if(partIndices.size() == 1)
        dir = allParts(partIndices[0]).applyNM(origDir);
    glm::vec3 pointToLine = -lineToPoint(ray, dragPt - camPos);
    vec3 absValueOrig = vec3(abs(origDir.x),abs(origDir.y), abs(origDir.z));
    float increase = glm::dot(dir, pointToLine);

    //change the scale for every 1 meter the user drags the scale part in the direction the scale part represents.
    if(fGEqual(abs(increase/0.25),1) && !fGEqual(abs(increase/0.25),1.9f))
    {
        if(scale)
        {
            // for(int i = 0; i < partIndices.size(); i++)
            // {
                if(partIndices.size() > 1)
                    return;
                PartData& part = allParts(partIndices[0]);
                PartData oldPD = part;
                vec3 ptOnPart = vec3(glm::transpose(part.rotationMatrix) * vec4(scalePart.translate - dir - part.translate, 1.0f));
                ptOnPart = vec3(part.rotationMatrix * vec4(ptOnPart/part.scaleVector, 1.0f));
                increase = (int)(increase/0.25) * 0.25;
                float scaleAdd = increase/glm::dot(dir,ptOnPart);
                vec3 newScale = (scaleAdd * absValueOrig) + part.scaleVector;
                if(fequal(newScale.x, 0.0f) || fequal(newScale.y, 0.0f) || fequal(newScale.z, 0.0f))
                    return;
                part.scale(newScale);
                part.translate = part.translate + (dir * increase);
                //check if part collides with any, if so, revert back to old scale.
                std::vector<int> touching;
               
                if(!draggedCheckCollide(partIndices[0], -1, touching, false))
                {
                    part = oldPD;
                }
                else
                {
                    dragPt = dragPt + (dir * increase * 2.0f);
                }
            // }
        }
        else
        {
            bool canMove = true;
            for(int i = 0; i < partIndices.size(); i++)
            {
                PartData& part = allParts(partIndices[i]);
                increase = (int)(increase/0.25) * 0.25;
                part.translate += dir * increase * 2.0f;
                //check if part collides with any, if so, revert back to old scale.
                std::vector<int> touching;
                if(draggedCheckCollide(partIndices[i], -1, touching, false) == false)
                {
                    for(int j = 0; j < touching.size(); j++)
                    {
                        //if the current part we are overcolliding with is not a selected part, we cannot move the selection
                        if(touching[j] < 0 && std::find(selectedParts.begin(), selectedParts.end(), abs(touching[j])) == selectedParts.end())
                        {
                            canMove = false;
                        }
                    }
                }
            }
            if(!canMove)
            {
                for(int i = 0; i < partIndices.size(); i++)
                {
                    PartData& part = allParts(partIndices[i]);
                    increase = (int)(increase/0.25) * 0.25;
                    part.translate -= dir * increase * 2.0f;
                }
            }
            else
            {
                dragPt = dragPt + (dir * increase * 2.0f);
            }
        }
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
    bool doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching, true);
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
        {
            std::cout << "NO NEGATIVES??\n";
            break;
        }
        else
            std::cout << "yes negatives\n";
        //now that we may have changed the position of the top part, we must check again if it is colliding with any other parts.
        doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching, true);
    }
}

glm::vec3 maxPtInDirMult(std::vector<int> partIndices, glm::vec3 dir)
{
    vec3 max(0,0,0);
    float maxDotDir = -FLT_MAX;
    for(int i = 0; i < partIndices.size(); i++)
    {
        vec3 currMax = maxPtInDir(partIndices[i], dir);
        float currDotDir = glm::dot(dir, currMax);
        if(currDotDir > maxDotDir || i == 0)
        {
            max = currMax;
            maxDotDir = currDotDir;
        }
    }
    return max;
}

void moveMultAbove(std::vector<int> topIndices, int bottomIndex)
{
    //to move multiple parts above another part, simply we find the lowest point on any of the mult parts
    //, then set the y position of this point to the top of the bottom part
    vec3 upVec = glm::vec3(0.0f,1.0f,0.0f);
    vec3 topOfBot = maxPtInDir(bottomIndex, upVec);
    vec3 botOfTopParts = maxPtInDirMult(topIndices, -upVec);
    float yDifference = glm::dot(upVec, (botOfTopParts - topOfBot));
    //if the two points are above, the top parts are guaranteed to be above the bottom part.
    if(yDifference > 0.0f)
    {
        return;
    }
    for(int i = 0; i < topIndices.size(); i++)
    {
        int t = topIndices[i];
        int b = bottomIndex;
        PartData& part1 = allParts(t);
        part1.translate += (upVec * (-0.01f - yDifference));
    }


    
}
//adjusts the position of multiple parts that have just been pasted so that they don't overcollide with any other parts
void adjustNewObjPosMult(std::vector<int>& topIndices, int bottomIndex, vector<int>& touching)
{
    //find the middle of the top parts and move parts so that middle aligns with bottom translate
    glm::vec3 topCenter = centerOfParts(topIndices);
    glm::vec3 offset = allParts(bottomIndex).translate - topCenter;
    for(int i = 0; i < topIndices.size(); i++)
        allParts(topIndices[i]).translate += offset;
    moveMultAbove(topIndices, bottomIndex);
    
    for(int i = 0; i < topIndices.size(); i++)
    {
        int topIndex = topIndices[i];
        bool negative = false;
        bool doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching, false);
        while(!doneAdjusting)
        {
            negative = false;
            for(int k = 0; k < touching.size(); k++)
            {
                int currIndex = touching[k];
                if(currIndex < 0)
                {
                    bottomIndex = abs(currIndex);
                    moveMultAbove(topIndices, abs(currIndex));
                    negative = true;
                    touching.clear();
                    i = 0;
                    break;
                }
            }
            if(!negative)
                std::cout << "NO NEGATIVES??\n";
            else
                std::cout << "yes negatives\n";
            //now that we may have changed the position of the top part, we must check again if it is colliding with any other parts.
            doneAdjusting = draggedCheckCollide(topIndex, bottomIndex, touching, false);

        }
        std::cout << "done adjusting\n";
    }
}
//add the a part of the current new type to the workspace.
void addPart()
{
    PartData& newPart = *(VAOs[currNewPart]->addPart());
    newPart.color = vec3(1.0f,0.0f,0.0f);
    vec3 selectedCenter = allParts(6).translate;
    if(selectedParts.size() > 0)
        selectedCenter = centerOfParts(selectedParts);

    //we set the position of this part to above the currently selected part.
    //if no parts are selected, set the position to above the ground part.
    //if there are parts above this bottom part, the highest part is the new bottom part (our new part is built on top of it)

    //we will use the partsrayintersects function to find the real bottom part.
    vec3 botVec = glm::vec3(0.0f, -1.0f, 0.0f);
    vec3 rayOrigin = selectedCenter + glm::vec3(0.0f,100.0f, 0.0f);
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
        combineParts(touching, topPart, false);
    newPart.color = currNewColor;
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
    newPart->color = currNewColor;
}

//copies all selected parts.
void copyParts()
{
    //dump part data that is in partsToCopy already
    partsToCopy.clear();
    partsToCopyTypes.clear();
    for(int i = 0; i < selectedParts.size(); i++)
    {
        partsToCopy.push_back(allParts(selectedParts[i]));
        partsToCopyTypes.push_back(pool[selectedParts[i]].type);
    }
}

glm::vec3 centerOfParts(std::vector<int>& partIndices)
{
    vec3 mid;
    for(int i = 0; i < partIndices.size(); i++)
    {
        mid += allParts(partIndices[i]).translate;
    }
    mid = mid/((float)partIndices.size());
    return mid;
}

//pastes copied parts into the world
void pasteParts()
{
    //find middle of selected parts, for later step
    vec3 selectedCenter = allParts(6).translate;
    if(selectedParts.size() > 0)
        selectedCenter = centerOfParts(selectedParts);
    //the pasted parts will be selected, clear what is in selection now
    selectedParts.clear();
    for(int i = 0; i < partsToCopy.size(); i++)
    {
        selectedParts.push_back(totalParts);
        //create new part
        PartData* newPart = VAOs[partsToCopyTypes[i]]->addPart();
        //copy old part data into new part
        (*newPart) = partsToCopy[i];
    }
    //now we must adjust the position of all the parts
    //--------------

    vec3 botVec = glm::vec3(0.0f, -1.0f, 0.0f);
    vec3 rayOrigin = selectedCenter + glm::vec3(0.0f,200.0f, 0.0f);
    vec3 intPt;
    int intNormIndex;
    int trueBottomPart = partsRayIntersects(true, botVec, rayOrigin, &intPt, &intNormIndex, false);
    std::cout << "TRUE BOTTOM PART IS " << trueBottomPart << '\n';
    vector<int> touching;
    int topPart = totalParts-1;
    adjustNewObjPosMult(selectedParts, trueBottomPart, touching);

    //done with above

    touching.insert(touching.end(), selectedParts.begin(), selectedParts.end());
    std::sort(touching.begin(), touching.end());
    touching.erase(std::unique(touching.begin(), touching.end()), touching.end());
    // touching.push_back(topPart);

    
    for(int i = 0; i < touching.size(); i++)
        if(allPhys(touching[i]).stationary == true)
            removeFromVec(touching, touching[i]);
    
    //now we add the objects that are touching to a group together.
    std::cout << "TOUCHING SIZE = " << touching.size() << '\n';
    if(touching.size() > 1)
        combineParts(touching, -1, true);
}

//combines parts into a group. if the parts are already in
//their own groups, combines the groups
//also, add each part to each others adjacency lists
//has slightly different behavior when there are multiple "dragged" parts.
void combineParts(std::vector<int>& p, int draggedPart, bool multipleDrag)
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
    if(!multipleDrag)
    {
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

    }
    //if multiple drag is true, the dragged part argument will be ignored
    else
    {
        for(int i = 0; i < p.size(); i++)
        {
            vector<int>& adj = pool[p[i]].nodeInfo.adjacent;
            adj.clear();
            for(int j = 0; j < totalParts; j++)
            {
                vec3 intData[3];
                if(SAT(p[i], j, intData) == true)
                {
                    adj.push_back(j);
                }
            }
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