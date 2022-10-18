#include "include/PhysicsData.h"
#include "include/VAOData.h"
#include "include/game.h"
#include <iostream>

struct lineSeg
{
    glm::vec3& p1;
    glm::vec3& p2;
};

glm::vec3 lineSegmentsIntersect(glm::vec3 p1Line1, glm::vec3 p2Line1, glm::vec3 p1Line2, glm::vec3 p2Line2)
{
    glm::vec3 s = (p2Line1 - p1Line1), r = (p2Line2 - p1Line2);
    glm::vec3 numerator = glm::cross((p1Line1 - p1Line2), s);
    glm::vec3 denominator = glm::cross(s, r);
    return numerator/denominator;
}

glm::vec3 lineIntersectPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 planeNormal, glm::vec3 planeOrigin)
{
    glm::vec3 vec = p2 - p1;
    glm::vec3 vecDir = glm::normalize(vec);
    float denom = glm::dot(vecDir, planeNormal);
    float numer = -(glm::dot((p1 - planeOrigin), planeNormal));
    float x = -numer*1.0f/denom;
    return x*vecDir;
}

//returns true if there is an intersection between p1 and p2
bool SAT(PartData p1, PartData p2, int p1Type, int p2Type)
{
    //must see if there is any gap between the objects on the axis of their normals
    VAOData* theVAOs[] = {VAOs[p1Type], VAOs[p2Type]};
    GLfloat* verts[] = {VAOs[p1Type]->VBData, VAOs[p2Type]->VBData};
    PartData parts[2] = {p1, p2};

    std::vector<glm::vec3> edgeNormals;
    std::vector<lineSeg> p1Edges; std::vector<lineSeg> p2Edges;
    //populate edgeNormals
    for(int one = 0; one < ((theVAOs[0]->VBDataSize/sizeof(GLfloat)) - 5); one+=5)
    {
        glm::vec3 vertOne1{verts[0][one], verts[0][one + 1], verts[0][one + 2]};
        glm::vec3 vertOne2{verts[0][one + 5], verts[0][one + 6], verts[0][one + 7]};
        for(int two = 0; two < ((theVAOs[1]->VBDataSize/sizeof(GLfloat)) - 5); two+=5)
        {
            glm::vec3 vertTwo1{verts[0][two], verts[0][two + 1], verts[0][two + 2]};
            glm::vec3 vertTwo2{verts[0][two + 5], verts[0][two + 6], verts[0][two + 7]};
            glm::vec3 planeNorm = glm::normalize(glm::cross(vertOne2 - vertOne1, vertTwo1 - vertTwo2));
            bool alreadyIn = false;
            for(int b = 0; b < edgeNormals.size(); b++) 
                if(edgeNormals[b] == planeNorm) 
                    alreadyIn = true;
            if(!alreadyIn)
            {
                edgeNormals.push_back(planeNorm);
                p1Edges.push_back({vertOne1, vertOne2});
                p2Edges.push_back({vertTwo1, vertTwo2});
            }
        }
    }


    glm::vec3 leastIntersectionNormal{0.0f,0.0f,0.0f};
    glm::vec3 p1LeastIntersectionPoints[2];
    glm::vec3 p2LeastIntersectionPoints[2];
    float leastIntersectionLength = 999999.0f;
    int leastIntersectionK;
    //loop thru normals
    for(int k = 0; k < 3; k++)
    {
        int numNormals;
        glm::vec3* theNormals;
        if(k != 2)
        {
            numNormals = theVAOs[k]->numNormals;
            theNormals = theVAOs[k]->normalsArray;
        }
        else
        {
            numNormals = edgeNormals.size();
            theNormals = (glm::vec3*)(&edgeNormals[0]);
        }
        for(int i = 0; i < numNormals; i++)
        {
            //for this normal, see which vertices of both objects are the least and most in direction of normal. The vector between these points will be the "shadow" that each object casts
            glm::vec3 normal = theNormals[i];
            if(k != 2)
                normal = parts[k].normalMatrix * normal;
            glm::vec3 highs[2]; glm::vec3 lows[2];
            float highVals[2]; float lowVals[2];
            //for loop to check both p1 verts and p2 verts
            for(int z = 0; z < 2; z++)
            {
                PartData currPart = parts[z];
                GLfloat* currVerts = verts[z]; 
                int currVertsSize = theVAOs[z]->VBDataSize/sizeof(GLfloat);
                glm::vec3 currHigh, currLow;
                float currLowVal, currHighVal;
                
                for(int j = 0; j < (currVertsSize)/5; j++)
                {
                    glm::vec3 currPoint = currPart.translate + (glm::vec3(currVerts[j * 5], currVerts[(j * 5) + 1], currVerts[(j * 5) + 2]));
                    float currProjection = glm::dot(normal, currPoint);
                    if(currProjection >= currHighVal || j == 0)
                    {
                        currHighVal = currProjection;
                        currHigh = currPoint;
                    }
                    if(currProjection <= currLowVal || j == 0)
                    {
                        currLowVal = currProjection;
                        currLow = currPoint;
                    }
                }
                highs[z] = currHigh;
                lows[z] = currLow;
                lowVals[z] = currLowVal;
                highVals[z] = currHighVal;
            }
            float combinedShadow = (highVals[0] - lowVals[0]) + (highVals[1] - lowVals[1]);
            int lowest = lowVals[0] < lowVals[1] ? 0 : 1;
            float stretchShadow = highVals[1-lowest] - lowVals[lowest];
            if(stretchShadow > combinedShadow)
                return false;

            //
        }
    }
    return true;
}

void cylinderIntersect()
{
    float radius = 0.5f;
    glm::vec3 p1{0.0f,0.25f,1.0f};
    glm::vec3 p2{1.0f,0.25f,1.0f};
    glm::vec3 p1Trim = p1;
    glm::vec3 p2Trim = p2;
    glm::vec3 bottomCylinder{0.0f, -0.5f, 0.0f};
    glm::vec3 topCylinder{0.0f, 0.5f, 0.0f};

    glm::vec3 bottomNormal = glm::normalize(topCylinder - bottomCylinder);
    glm::vec3 topNormal = -bottomNormal;

    bool p1InBot = glm::dot(p1,bottomNormal) > glm::dot(bottomCylinder, bottomNormal);
    bool p1InTop = glm::dot(p1,topNormal) > glm::dot(topCylinder, topNormal);
    bool p2InBot = glm::dot(p2,bottomNormal) > glm::dot(bottomCylinder, bottomNormal);
    bool p2InTop = glm::dot(p2,topNormal) > glm::dot(topCylinder, topNormal);


    //if both points are outside either top or bottom normal, then we don't intersect
    if((!p1InBot && !p2InBot) || (!p2InTop && !p1InTop))
    {
        std::cout << "no intersection\n";
    }

    //first we trim the line
    if(!p1InTop)
    {
        p1Trim = lineIntersectPlane(p2Trim, p1Trim, topNormal, topCylinder);
    }
    if(!p1InBot)
    {
        p1Trim = lineIntersectPlane(p2Trim, p1Trim, bottomNormal, bottomCylinder);
    }
    if(!p2InBot)
    {
        p2Trim = lineIntersectPlane(p1Trim, p2Trim, bottomNormal, bottomCylinder);
    }
    if(!p2InTop)
    {
        p2Trim = lineIntersectPlane(p1Trim, p2Trim, topNormal, topCylinder);
    }

    //now we map the line and cylinder points to 2d
    glm::vec3 p1_2D = p1Trim - (bottomNormal * (glm::dot(p1Trim, bottomNormal)));
    glm::vec3 p2_2D = p2Trim - (bottomNormal * (glm::dot(p2Trim, bottomNormal)));
    glm::vec3 botCyl_2D = bottomCylinder - (bottomNormal * (glm::dot(bottomCylinder, bottomNormal)));
    glm::vec3 vec_2D = p2_2D - p1_2D;

    float p1InVecDir = glm::dot(p1_2D, vec_2D);
    float p2InVecDir = glm::dot(p2_2D, vec_2D);
    float cylPointInVecDir = glm::dot(botCyl_2D, vec_2D);
    
    glm::vec3 closestPoint{999.0f,999.0f,999.0f};
    if(p1InVecDir > cylPointInVecDir)
    {
        closestPoint = p1_2D;
    }
    else if(p2InVecDir < cylPointInVecDir)
    {
        closestPoint = p2_2D;
    }
    //closest point between p1 and p2 (2D)
    else
    {
        glm::vec3 p1ToBot = botCyl_2D - p1_2D;
        closestPoint = p1_2D + glm::dot(p1ToBot, glm::normalize(vec_2D));
    }
    if(glm::length(closestPoint - botCyl_2D) <= radius)
    {
        std::cout << "INTERSECTION\n";
    }
    else
    {
        std::cout << "NO INTERSECTION (end)\n";
    }
    std::cout << "CLOSEST POINT IS " << closestPoint.x << ", " << closestPoint.y << ", " << closestPoint.z << '\n';
    std::cout << "botCyl_2D is " << botCyl_2D.x << ", " << botCyl_2D.y << ", " << botCyl_2D.z << '\n';
}


//in progress
void physicsTick()
{   //for all VAOs:
        //for all parts:
            //gravity
            //check collision
    
    for(int i = 0; i < totalParts; i++)
    {
        PartData& part1 = *(partPool[i]);
        PhysicsData& physics1 = *(physicsPool[i]);
        //apply acceleration (also gravity)
        physics1.linearV += physics1.linearA;
        physics1.angularV += physics1.angularA;

        //check for collision with every other part
        float p1Max = std::max(std::max(part1.scaleVector.x, part1.scaleVector.y), part1.scaleVector.z);
        for(int j = i+1; j < totalParts; j++)
        {
            PartData& part2 = *(partPool[j]);
            float p2Max = std::max(std::max(part2.scaleVector.x, part2.scaleVector.y), part2.scaleVector.z);
            if(glm::length(part1.translate - part2.translate) > (p2Max + p1Max))
                //it is not possible for p1 and p2 to intersect
                continue;

            //if we have reached this point, we check for intersection
            bool result = SAT(part1, part2, partTypes[i], partTypes[j]);
            
        }
        //mark part1 as checked
        // checkedPart[] = true;

    }
    

}