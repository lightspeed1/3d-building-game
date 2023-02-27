#include "include/PhysicsData.h"
#include "include/VAOData.h"
#include "include/game.h"
#include "include/DrawPrims.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace std::chrono;

using namespace glm;
using namespace std;
//floats equal
bool fequal(float a, float b)
{
    return(abs(a-b) < 0.00001);
}
bool fGEqual(float a, float b)
{
    return(a-b > 0 || fequal(a,b));
}
bool fGButNotEqual(float a, float b)
{
    return(a-b > 0 && !fequal(a,b));
}
bool v3Equal(glm::vec3 one, glm::vec3 two)
{
    for(int c = 0; c < 3; c++)
    {
        bool res = fequal(one[c], two[c]);
        if(!res)
            return false;
    }
    return true;
}
bool pairCmp(pair<float,vec3> a, pair<float,vec3> b)
{
    return(a.first < b.first);
}

struct lineSeg
{
    glm::vec3 start;
    glm::vec3 end;
};
//returns 1 if they intersect, -1 if they do not, and 0 if they are on the same line.
//this function is used to check if the edges of two objects are colliding
int lineSegmentsIntersect(glm::vec3 p1Line1, glm::vec3 p2Line1, glm::vec3 p1Line2, glm::vec3 p2Line2, glm::vec3* intPts)
{
    glm::vec3 s = (p2Line1 - p1Line1), r = (p2Line2 - p1Line2);
    glm::vec3 numerator = glm::cross((p1Line2 - p1Line1), r);
    glm::vec3 denominator = glm::cross(s, r);
    int numSame = 0;
    if(v3Equal(p1Line1, p1Line2))
        numSame++;
    else if(v3Equal(p1Line1, p2Line2))
        numSame++;
    if(v3Equal(p2Line1, p1Line2))
        numSame++;
    else if(v3Equal(p2Line1, p2Line2))
        numSame++;
    if(numSame >= 2)
        return 0;
    //if lines are parallel, (cross product is 0), then we must check how much of the segments are overlapping.
    if(v3Equal(denominator, glm::vec3(0,0,0)))
    {
        if(v3Equal(numerator, glm::vec3(0,0,0)))
        {
            glm::vec3 n = glm::normalize(p2Line1 - p1Line1);
            vector<pair<float,vec3>> projs = {
                pair<float,vec3>{dot(n,p1Line1),p1Line1},
                pair<float,vec3>{dot(n,p2Line1),p2Line1},
                pair<float,vec3>{dot(n,p1Line2),p1Line2},
                pair<float,vec3>{dot(n,p2Line2),p2Line2},
            };
            if((fGEqual(projs[0].first,projs[2].first) && fGEqual(projs[3].first,projs[0].first))
            || (fGEqual(projs[2].first,projs[0].first) && fGEqual(projs[1].first,projs[2].first)))
            {
                //now we know that the lines overlap eachother
                std::sort(projs.begin(),projs.end(), pairCmp);
                intPts[0] = projs[1].second; intPts[1] = projs[2].second;
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    float u = glm::length(numerator)/glm::length(denominator);
    vec3 numerator2 = cross((p1Line1 - p1Line2), s);
    vec3 denominator2 = cross(r,s);
    float t = glm::length(numerator2)/glm::length(denominator2);
    vec3 b = numerator2/denominator2;
    glm::vec3 intPoint = p1Line1 + (u * s);
    vec3 intPoint2 = p1Line2 + (t * r);
    //the calculated intersection point must be the same whether we are moving from the origin of segment 1 or segment 2.
    if(v3Equal(intPoint2,intPoint) == false || t > 1.0f || u > 1.0f)
    {
        return -1;
    }    
    intPts[0] = intPoint;
    //now we know that segments are intersecting at a single point, which means they do not lie on the same line
    return 1;
}


//checks if a point is in a 2D shape
bool inShape(vector<vec3>& shapePoints, vector<pair<int,int>> shapeEdges, vec3 point)
{
    //if the segment of the point to the center of the shape intersects any of the shape edges, we know the point is outside of the shape
    vec3 shapeCenter = std::accumulate(shapePoints.begin(),shapePoints.end(),glm::vec3(0,0,0))/((float)shapePoints.size());
    for(int i = 0; i < shapeEdges.size(); i++)
    {
        vec3 first = shapePoints[shapeEdges[i].first], second = shapePoints[shapeEdges[i].second];
        vec3 intPts[2];
        int result = (lineSegmentsIntersect(first,second,point,shapeCenter,intPts));
        int numSame = 0;
        if(v3Equal(first, point))
            numSame++;
        else if(v3Equal(first, shapeCenter))
            numSame++;
        if(v3Equal(second, point))
            numSame++;
        else if(v3Equal(second, shapeCenter))
            numSame++;
        if(result == 1 && numSame == 0)
        {
            return false;
        }
    }
    return true;
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
//finds a vec3 within a std::vector. We must make our own function because vec3 contains floats and we cannot just use the == operator to check if two floats are the same.
int find(std::vector<glm::vec3>& arr, glm::vec3 thing)
{
    float* thingFloats = (float*)(&(thing.x));
    int index = -1;
    for(int i = 0; i < arr.size(); i++)
    {
        float* currFloats = (float*)(&(arr[i].x));
        bool in = true;
        for(int j = 0; j < 3; j++)
            if(abs((currFloats[j]) - (thingFloats[j])) > 0.001)
                in = false;
        if(in)
        {
            index = i;
            break;
        }
    }
    return index;
}

//checks if point is in a 3D object
bool pointInPart(int partIndex, vec3 pt)
{
    PartData& part = allParts(partIndex);
    VAOData& currVAO = *(VAOs[pool[partIndex].type]);
    //loop through normals and see if a point on that face of the normal is
    //-further along the normal than the pt we are testing
    bool in = true;
    for(int i = 0; i < currVAO.numNormals; i++)
    {
        float ptDot = glm::dot(currVAO.normalsArray[i], pt);
        float faceDot = glm::dot(currVAO.normalsArray[i], currVAO.ptsOnNorms[i]);
        if(fGButNotEqual(ptDot, faceDot))
        {
            in = false;
            break;
        }
    }
    return in;
}

//sorts vector of points so that adjacent points are connected to eachother and form an edge.
//this function assumes that each point is part of two edges on a shape, and that the others that share these edges are the 2 closest to the current point.
void sortPointsArr(vector<vec3>& arr, vector<pair<int,int>>& result)
{
    //this vector tells which two other points the corresponding point should be connected to.
    vector<pair<int,int>> ptPairs(arr.size(), pair<int,int>(-1,-1));
    for(int i = 0; i < ptPairs.size(); i++)
    {
        float minDist1 = INT_MAX, minDist2 = INT_MAX;
        for(int j = 0; j < ptPairs.size(); j++)
        {
            if(i == j) continue;
            float currDist = length(arr[i] - arr[j]);
            if(fGButNotEqual(minDist1, currDist))
            {
                ptPairs[i].second = ptPairs[i].first; ptPairs[i].first = j;
                minDist2 = minDist1;
                minDist1 = currDist;
            }
            else if(fGButNotEqual(minDist2, currDist))
            {
                ptPairs[i].second = j;
                minDist2 = currDist;
            }
        }
    }
    int b = 1;
    //now we loop through ptPairs again to form pairs and eliminate useless pts
    for(int i = 0; i < ptPairs.size(); i++)
    {
        int ind1 = ptPairs[i].first, ind2 = ptPairs[i].second;
        pair<int,int> firstAdjInd = ptPairs[ind1];
        pair<int,int> secondAdjInd = ptPairs[ind2];
        if(firstAdjInd.first == i || firstAdjInd.second == i)
        {
            if(std::find(result.begin(),result.end(),pair<int,int>{i,ind1})==result.end()
            && std::find(result.begin(),result.end(),pair<int,int>{ind1,i})==result.end())
            {
                result.push_back(pair<int,int>{i,ind1});
                ptPairs[i].first = -1;
            }
        }
        if(secondAdjInd.first == i || secondAdjInd.second == i)
        {
            if(std::find(result.begin(),result.end(),pair<int,int>{i,ind2})==result.end()
            && std::find(result.begin(),result.end(),pair<int,int>{ind2,i})==result.end())
            {
                result.push_back(pair<int,int>{i,ind2});
                ptPairs[i].second = -1;
            }
        }
    }
}

//returns true if there is an intersection between p1 and p2. which are 3D objects
bool SAT(int p1Index, int p2Index, glm::vec3* intersectionData)
{
    PartData& p1 = allParts(p1Index), & p2 = allParts(p2Index);
    PhysicsData p1Phys = allPhys(p1Index), p2Phys = allPhys(p2Index);
    if(p2Index < 0 || p2Index > totalParts)
    {
        return false;
    }
    int p1Type = pool[p1Index].type;
    int p2Type = pool[p2Index].type;
    // std::cout << p1Type << " " << p2Type << '\n';
    //must see if there is any gap between the objects on the axis of their normals
    VAOData* theVAOs[] = {VAOs[p1Type], VAOs[p2Type]};
    GLfloat* verts[] = {VAOs[p1Type]->VBData, VAOs[p2Type]->VBData};
    PartData parts[2] = {p1, p2};
    vector<unsigned int> edges[2] = {(theVAOs[0]->partEdges), (theVAOs[1]->partEdges)};
    vector<vec3> edgeNormals;

    vector<vector<vec3>> edgeNormalp1Indices;
    vector<vector<vec3>> edgeNormalp2Indices;
    vector<vector<vec3>>* edgeNormalIndices[2] = {&edgeNormalp1Indices,&edgeNormalp2Indices};
    
    //in these loops we are creating normals from the cross product of every edge of p1 and p2
    //-we will have to test these normals as well as the normals that correspond to the
    //-faces of both objects later.
    for(int one = 0; one < edges[0].size() - 1; one+=2)
    {
        vec3 vertOne1 = p1.localToWorld(*((glm::vec3*)&verts[0][6 * edges[0][one]]));
        vec3 vertOne2 = p1.localToWorld(*((glm::vec3*)&verts[0][6 * edges[0][one + 1]]));
        vec3 vec1 = vertOne2 - vertOne1;

        for(int two = 0; two < edges[1].size() - 1; two+=2)
        {
            // vec3 vertTwo1 = p2.translate + (mat3(p2.rotationMatrix) * (p2.scaleVector * (*((vec3*)&verts[1][5 * edges[1][two]]))));
            vec3 vertTwo1 = p2.localToWorld((*((vec3*)&verts[1][6 * edges[1][two]])));
            // vec3 vertTwo2 = p2.translate + (mat3(p2.rotationMatrix) * (p2.scaleVector * (*((vec3*)&verts[1][5 * edges[1][two + 1]]))));
            vec3 vertTwo2 = p2.localToWorld((*((vec3*)&verts[1][6 * edges[1][two + 1]])));
            vec3 vec2 = vertTwo2 - vertTwo1;
            vec3 planeNorm = (glm::cross(vec1, vec2));
            if(planeNorm == vec3(0,0,0))
                continue;
            
            planeNorm = glm::normalize(planeNorm);
            if(planeNorm.x == 0) planeNorm.x = 0;
            if(planeNorm.y == 0) planeNorm.y = 0;
            if(planeNorm.z == 0) planeNorm.z = 0;
            //if edgenormal is not found in list
            int index;
            if((index = find(edgeNormals, planeNorm)) == -1)
            {
                edgeNormals.push_back(planeNorm);
                edgeNormalp1Indices.push_back(std::vector<glm::vec3>{vertOne1,vertOne2});
                edgeNormalp2Indices.push_back(std::vector<glm::vec3>{vertTwo1, vertTwo2});
            }
            else
            {
                edgeNormalp1Indices[index].push_back(vertOne1); edgeNormalp1Indices[index].push_back(vertOne2);
                edgeNormalp2Indices[index].push_back(vertTwo1); edgeNormalp2Indices[index].push_back(vertTwo2);
            }
        }
    }
    vec3 leastIntersectionNormal{0.0f,0.0f,0.0f};
    //low, high    v
    vec3 leastIntersectionPointsLow[2];
    vec3 leastIntersectionPointsHigh[2];
    float leastIntersectionValsLow[2];
    float leastIntersectionValsHigh[2];
    float leastIntersectionLength = 999999.0f;
    int leastIntersectionK;
    // std::cout << "LEAST INT K: " << leastIntersectionK << '\n';
    int leastIntSamePointsNormSide = 0;
    int leastIntSamePointsOtherSide = 0;
    int normBelongs = 0;
    vector<vec3> normSidePoints;
    vector<vec3> otherSidePoints;
    int edgeNormIndex = -1;
    //loop thru normals
    for(int k = 0; k < 3; k++)
    {
        int numNormals;
        vec3* theNormals;
        if(k != 2)
        {
            numNormals = theVAOs[k]->numNormals;
            theNormals = theVAOs[k]->normalsArray;
        }
        else
        {
            numNormals = edgeNormals.size();
            theNormals = (vec3*)(&(edgeNormals[0]));
        }
        for(int i = 0; i < numNormals; i++)
        {
            //for this normal, see which vertices of both objects are the least and most in direction of normal. The vector between these points will be the "shadow" that each object casts
            vec3 normal = theNormals[i];
            int normBelongsTo = k;
            if(k != 2)
                normal = parts[k].applyNM(normal);
            else
                normBelongsTo = glm::dot(normal, p1.translate) > glm::dot(normal, p2.translate) ? 1 : 0;
            vec3 highs[2]; vec3 lows[2];
            float highVals[2]; float lowVals[2];
            int sameDotNorm = 1, sameDotOther = 1;
            vector<vec3> currNormSidePoints;
            vector<vec3> currOtherSidePoints;
            //for loop to check both p1 verts and p2 verts
            for(int z = 0; z < 2; z++)
            {
                PartData currPart = parts[z];
                GLfloat* currVerts = verts[z]; 
                int currVertsSize = theVAOs[z]->VBDataSize/sizeof(GLfloat);
                vec3 currHigh, currLow;
                
                float currLowVal = 9999999.0f, currHighVal = -1.0f;
                
                for(int j = 0; j < edges[z].size(); j+=2)
                {
                    // vec3 currPoint = currPart.localToWorld(*((vec3*)(currVerts + (j*6))));
                    vec3 currPoint = currPart.localToWorld(*((glm::vec3*)&verts[z][6 * edges[z][j]]));
                    float currProjection = (glm::dot(normal, currPoint));
                    if(currProjection > currHighVal || fequal(currProjection, currHighVal) || j == 0)
                    {
                        
                        //if this point is equally along the normal as the previous point, the points are on the same face
                        if(j != 0 && (z == k || (z == normBelongsTo && k == 2)))
                        {
                            int index = find(currNormSidePoints, currPoint);
                            if(index == -1)
                            {
                                if(fequal(currProjection, currHighVal))
                                {
                                    currNormSidePoints.push_back(currPoint);
                                }
                                else
                                {
                                    currNormSidePoints.clear();
                                }
                            }
                        }
                        currHighVal = currProjection;
                        currHigh = currPoint;

                    }
                    
                    if(currProjection < currLowVal || fequal(currProjection, currLowVal) || j == 0)
                    {
                        if(j != 0 && ((z != k && k != 2) || (z != normBelongsTo && k == 2)))
                        {
                            int index = find(currOtherSidePoints, currPoint);
                            if(index == -1)
                            {
                                if(fequal(currProjection, currLowVal))
                                {
                                    currOtherSidePoints.push_back(currPoint);
                                }
                                else
                                {
                                    currOtherSidePoints.clear();
                                }
                            }
                        }
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


            //now that we know this normal has no gap in the shadow, we see if it is the normal where the objects initially collided.
            vec3 overlap = normal * (combinedShadow - stretchShadow);
            //we want overlap to be low as possible. to attempt to make it even lower, we take away however much of it is in the direction of the velocity so that the normals where the objects most likely collided have very low overlap.
            vec3 adjustedOverlap = normal - (normal * glm::dot(overlap,p1Phys.linearV - p2Phys.linearV)); 
            float adjustedOverlapLen = glm::length(overlap);
            
            //we also need to check if the normal is pointing the right direction
            //normal belongs to object k
            bool rightDir = (k == 2 || lowest == k);
            if(leastIntersectionLength > adjustedOverlapLen && rightDir)
            {
                leastIntersectionLength = adjustedOverlapLen;
                leastIntersectionNormal = normal;
                leastIntersectionPointsHigh[0] = highs[0]; leastIntersectionPointsHigh[1] = highs[1]; 
                leastIntersectionPointsLow[0] = lows[0]; leastIntersectionPointsLow[1] = lows[1];
                leastIntersectionValsHigh[0] = highVals[0]; leastIntersectionValsHigh[1] = highVals[1]; 
                leastIntersectionValsLow[0] = lowVals[0]; leastIntersectionValsLow[1] = lowVals[1]; 
                leastIntersectionK = k;
                // std::cout << "found k: " << k << '\n';
                edgeNormIndex = i;
                leastIntSamePointsNormSide = sameDotNorm; leastIntSamePointsOtherSide = sameDotOther;
                otherSidePoints = currOtherSidePoints; normSidePoints = currNormSidePoints;
                normBelongs = normBelongsTo;
            }
        }
    }
    //now we use the normal of intersection to find the point of intersection.

    //if k == 2, then an edge normal is the normal of intersection 
    int k1 = leastIntersectionK;
    // if(!(k1 >= 0 && k1 <= 2))
    // {
    //     k1 = 0;
    // }
    // std::cout << "here is k1: " << k1 << '\n';

    vec3 norm = leastIntersectionNormal;
    vec3 v;

    //if there is a face on collision, we take the intersection point as the center of the shared area between the faces.
    if((otherSidePoints.size() >= 3 && normSidePoints.size() >= 2) || (otherSidePoints.size() >= 2 && normSidePoints.size() >= 3)) 
    {
        vec3 otherProjAmount = norm * glm::dot(norm, otherSidePoints[0]);
        vec3 normProjAmount = norm * glm::dot(norm, normSidePoints[0]);
        vector<vec3> otherPtsProj = otherSidePoints, normPtsProj = normSidePoints;

        for(vec3& vN: otherPtsProj)
        {
            vN = vN - (norm * glm::dot(norm,vN));
            continue;
        }
        for(vec3& vO: normPtsProj)
        {
            vO = vO - (norm * glm::dot(norm, vO));
            continue;
        }
        //now we need to sort the points in the two vectors above so each pair of adjacent points
        //-in the vector actually forms an edge on the shape.
        vector<pair<int,int>> normFaceEdges, otherFaceEdges;
        sortPointsArr(normPtsProj, normFaceEdges);sortPointsArr(otherPtsProj, otherFaceEdges);

        vector<vec3> allIntPts;
        bool checkedOtherInShape = false;
        //for each edge in both lists, find if they intersect. if so, add the intersection point to a list.
        // std::cout << "normPtsProj is same as normFaceEdges" << normPtsProj.size() << normFaceEdges.size() << '\n';
        // std::cout << "otherPtsProj is same as otherFaceEdges" << otherPtsProj.size() << otherFaceEdges.size() << '\n';
        for(int i1 = 0; i1 < normFaceEdges.size(); i1++) {

            if(inShape(otherPtsProj, otherFaceEdges, normPtsProj[i1]) && find(allIntPts, normPtsProj[i1]) == -1)
            {
                allIntPts.push_back(normPtsProj[i1]);   
                // std::cout << "INSHAPE\n";
            }
            for(int i2 = 0; i2 < otherFaceEdges.size(); i2++) 
            {
                if(!checkedOtherInShape)
                {
                    if(inShape(normPtsProj, normFaceEdges, otherPtsProj[i2]) && find(allIntPts, otherPtsProj[i2]) == -1)
                    {
                        allIntPts.push_back(otherPtsProj[i2]);
                    }
                }
                vec3 currIntPts[2];
                vec3 first1 = normPtsProj[normFaceEdges[i1].first], second1 = normPtsProj[normFaceEdges[i1].second];
                vec3 first2 = otherPtsProj[otherFaceEdges[i2].first];
                vec3 second2 = otherPtsProj[otherFaceEdges[i2].second];
                int intResult = lineSegmentsIntersect(first1, second1, first2, second2, currIntPts);
                if(intResult == -1)
                    continue;
                else if(intResult == 1)
                {
                    //segments have one overlapping point
                    int index = find(allIntPts,currIntPts[0]);
                    if(index == -1)
                        allIntPts.push_back(currIntPts[0]);
                }
                else if(intResult == 0)
                {
                    //segments overlap eachother (two points were returned)
                    int index1 = find(allIntPts,currIntPts[0]), index2 = find(allIntPts,currIntPts[1]);
                    if(index1 == -1)
                        allIntPts.push_back(currIntPts[0]);
                    if(index2 == -1)
                        allIntPts.push_back(currIntPts[1]);
                }
            }
            checkedOtherInShape = true;
        }
        //now we find the center of mass of the shape that all intersection points create. (just average the points)
        // for(vec3& pt : allIntPts)
        //     pt += otherProjAmount;
        vec3 COM =  (std::accumulate(allIntPts.begin(),allIntPts.end(),vec3(0,0,0)))/((float)allIntPts.size());
        int other = 1 - k1;
        intersectionData[other] = COM + otherProjAmount;
        intersectionData[k1] = COM + normProjAmount;
    }
    else
    {
        if(k1 == 2)
        {
            // std::cout << "k1 = 2\n";
            
            // v   This is the part that has the lowest value
            //     along the normal. If it is 0, lowest is p1, else it is p2
            int lowest = leastIntersectionValsLow[0] < leastIntersectionValsLow[1] ? 0 : 1;
            int highest = 1 - lowest;
            //part 1 edge point 1
            vec3 p1Edgep1 = lowest == 0 ? leastIntersectionPointsHigh[0] : leastIntersectionPointsLow[0];
            vec3 p2Edgep1 = lowest == 1 ? leastIntersectionPointsHigh[1] : leastIntersectionPointsLow[1];
            vec3 p1Edgep2 = vec3(9.0f,9.0f,9.0f);
            vec3 p2Edgep2;
            
            //now find the other points used for the edges for the normal plane (by cross product).
            //start with lowest
            vec3 lowPartClosePoint = lowest == 1 ? leastIntersectionPointsHigh[1] : leastIntersectionPointsHigh[0];
            vector<vec3> lowestPartIndices = (*(edgeNormalIndices[lowest]))[edgeNormIndex];
            for(int l = 0; l < lowestPartIndices.size(); l+=2)
            {
                vec3 currPoint1 = lowestPartIndices[l];
                vec3 currPoint2 = lowestPartIndices[l+1];
                if(currPoint1 != lowPartClosePoint && currPoint2 != lowPartClosePoint)
                    continue;
                vec3 newPoint = currPoint1 == lowPartClosePoint ? currPoint2 : currPoint1;
                if(abs(glm::dot(norm,newPoint) - glm::dot(norm, lowPartClosePoint)) < 0.0001)
                {
                    //got other point
                    if(lowest == 1)
                        p2Edgep2 = newPoint;
                    if(lowest == 0)
                        p1Edgep2 = newPoint;
                }
            }

            //now highest
            vec3 highPartClosePoint = highest == 1 ? leastIntersectionPointsLow[1] : leastIntersectionPointsLow[0];
            vector<vec3> highestPartIndices = (*(edgeNormalIndices[highest]))[edgeNormIndex];
            for(int l = 0; l < highestPartIndices.size(); l+=2)
            {
                vec3 currPoint1 = highestPartIndices[l];
                vec3 currPoint2 = highestPartIndices[l+1];
                if(currPoint1 != highPartClosePoint && currPoint2 != highPartClosePoint)
                    continue;
                vec3 newPoint = currPoint1 == highPartClosePoint ? currPoint2 : currPoint1;
                if(abs(glm::dot(norm,newPoint) - glm::dot(norm, highPartClosePoint)) < 0.0001)
                {
                    //got other point
                    if(highest == 1)
                        p2Edgep2 = newPoint;
                    if(highest == 0)
                        p1Edgep2 = newPoint;
                }
            }
            //take the line segments that formed this normal and project them onto a plane, then we can find where they intersect
            lineSeg p1Seg = {p1Edgep1, p1Edgep2}; 

            lineSeg p2Seg = {p2Edgep1, p2Edgep2}; 
            lineSeg p1SegProjected = {p1Seg.start - (norm * glm::dot(norm, p1Seg.start)), p1Seg.end - (norm * glm::dot(norm, p1Seg.end))};
            lineSeg p2SegProjected = {p2Seg.start - (norm * glm::dot(norm, p2Seg.start)), p2Seg.end - (norm * glm::dot(norm, p2Seg.end))};

            vec3 intersectionPoint;
            lineSegmentsIntersect(p1SegProjected.start, p1SegProjected.end, p2SegProjected.start, p2SegProjected.end, &intersectionPoint);
            // float halfWay = abs(glm::dot(norm, p1Seg.start - p2Seg.start)/2.0f);
            // v   collision point for part 1
            v = intersectionPoint + (norm * glm::dot(norm, p1Seg.start));
            vec3 p2CollisionPoint = intersectionPoint + (norm * glm::dot(norm, p2Seg.start));
            intersectionData[0] = v;
            intersectionData[1] = p2CollisionPoint;

        }
        //if k!=2, then a part normal is the normal of intersection
        else
        {
            // std::cout << "k != 2\n";
            //find how much the part that doesn't possess the normal is intersecting the normal.
            int other = 1 - k1;
            vec3 penPoint = leastIntersectionPointsLow[other];
            
            vec3 planePoint = leastIntersectionPointsHigh[k1];
            //now we find how close the penetrating point is to the plane
            float planeDist = glm::dot(norm, planePoint);
            float penPointDist = glm::dot(norm, penPoint);
            float penToPlane = -(penPointDist - planeDist);
            //NOTE: we say that the pen part feels a force on an edge, and that the plane part feels force on a side (we must take away velocity perpindicular to plane)
            v = penPoint + (norm * penToPlane); 
            vec3 penPartCollisionPoint = penPoint;
            intersectionData[other] = penPartCollisionPoint;
            intersectionData[k1] = v;
        }
    }

    if(gameMode == FREEZE)
        return true;
    float velInDir1 = glm::dot(norm, intersectionData[0]);
    float velInDir2 = glm::dot(norm, intersectionData[1]);
    vec3 oneToTwo = intersectionData[1] - intersectionData[0];
    vec3 twoToOne = -oneToTwo;
    vec3 oneMove = vec3(0,0,0), twoMove = vec3(0,0,0);
    if(!fequal(velInDir1+velInDir2, 0.0f))
    {
        if(p1Index != 6 && p2Index != 6)
        {
            // oneMove = oneToTwo * (0.005f + (velInDir1)/(velInDir1+velInDir2));
            oneMove = oneToTwo * (1.0f);
            // twoMove = twoToOne * (0.005f + (velInDir2)/(velInDir1+velInDir2));
        }
        else
        {
            if(p1Index == 6)
                twoMove = twoToOne * (1.01f);
            else if(p2Index == 6)
                oneMove = oneToTwo * (1.01f);
        }
    }
    // p1.translate += oneMove; p2.translate += twoMove;
    group* p1GroupInd = pool[p1Index].groupPtr, *p2GroupInd = pool[p2Index].groupPtr;
    if(p1GroupInd != NULL) {
        group& currGroup = *p1GroupInd;
        for(int i = 0; i < currGroup.members.size(); i++)
            allParts(currGroup.members[i]).translate += oneMove;
    }
    else p1.translate += oneMove;
    if(p2GroupInd != NULL) {
        group& currGroup = *p2GroupInd;
        for(int i = 0; i < currGroup.members.size(); i++)
            allParts(currGroup.members[i]).translate += twoMove;
    }
    else p2.translate += twoMove;
    // std::cout << "MOVING BOTH BY: "; printVec3(oneMove); printVec3(twoMove);
    intersectionData[2] = norm;
    // std::cout << "same points norm, other: " << normSidePoints.size() << " " << otherSidePoints.size() << '\n';
    // for(int i = 0; i < normSidePoints.size(); i++)
    //     printVec3(normSidePoints[i]);
    // std::cout << "------------------------\n";
    // for(int i = 0; i < otherSidePoints.size(); i++)
    //     printVec3(otherSidePoints[i]);
    // printVec3(intersectionPoint_);
    return true;
}

inline float square(float input)
{
    return (input * input);
}

//returns the final velocity of two parts after they collide given their initial masses and velocities
std::vector<float> finalVelocities(float m1, float m2, float v1, float v2, bool p1Still, bool rotation, bool output)
{
    float cr = 0.1f;
    // float finalVb1 = ((m1*v1) + (m2*v2) - (m1*g))/(m2+m1);
    // float finalVa1 = ((m1*v1) + (m2*v2) - (m2 * finalVb1))/(m1);
    // float finalVa2 = ((m1*v1) + (m2*v2) - (m2*g))/(m2+m1);
    // float finalVb2 = ((m1*v1) + (m2*v2) - (m1 * finalVa2))/(m2);
    std::vector<float> result = {-1,-1};
    float g = cr * abs(v1 - v2);
    float finalVb1 = ((m1*v1) + (m2*v2) - (m1*g))/(m2+m1);
    float finalVa1 = ((m1*v1) + (m2*v2) - (m2 * finalVb1))/(m1);

    float finalVa2 = ((m1*v1) + (m2*v2) - (m2*g))/(m2+m1);
    float finalVb2 = ((m1*v1) + (m2*v2) - (m1 * finalVa2))/(m2);

    //make sure we don't return the old velocities (which would still technically work with momentum equation)
    // if(p1Still && rotation && output)
    // {
    //     std::cout << v1 << " " << v2 << '\n';
    //     // v1 = 0.0f;
        
    // }  
    
    if((v1 >= v2 && finalVb1 >= finalVa1) || (v2 >= v1 && finalVa1 >= finalVb1))
    {
        result[0] = finalVa1; result[1] = finalVb1;
        // if(rotation && output)
        //     std::cout << "1\n";
    }
    else
    {
        // if(rotation && output)
        //     std::cout << "2\n";
        result[0] = finalVa2; result[1] = finalVb2;
    }
    // if(p1Still && !rotation)
    // {

    //     std::cout << result[0] << " " << result[1] << " result\n";
    // }
    if(p1Still && rotation)
    {
        cr = 0.6f;
        g = cr * abs(v1 - v2);
        finalVa2 = ((m1*v1) + (m2*v2) - (m2*g))/(m2+m1);
        result[0] = finalVa2; result[1] = ((m1*v1) + (m2*v2) - (m1 * finalVa2))/(m2);
    }
    // if(p1Still && rotation && output)
    // {
    //     // result[0] = 0.0f;
    //     // result[1] = ((m1*v1) + (m2*v2))/(m2);
    //     // result[1] = ((m1*v1) + (m2*v2) - (m1*cr * abs(v2)))/(m2);
    //     std::cout << result[0] << " " << result[1] << " result\n";
    // }
    return result;
    
}

//If normalBelongs to is 0 or 1, it means the normal argument corresponds to part 1 and part 2 respectively
//This means that one object is being struck on a side, and another on an edge.
//If normalBelongsTo is -1, both objects are considered to have collided on an edge.
int cols = 0;
void partsCollide(int p1Index, int p2Index, glm::vec3 p1PointMass, glm::vec3 p2PointMass, glm::vec3 normal, glm::vec3* newVels)
{
    float mu = 0.5;
    float tickTime = 1.0f/60.0f;
    PhysicsData& p1Phys = allPhys(p1Index);
    PhysicsData& p2Phys = allPhys(p2Index);
    PartData& p1 = allParts(p1Index);
    PartData& p2 = allParts(p2Index);

    bool p1HasNormal = glm::dot(normal, p1PointMass) < glm::dot(normal,p2PointMass);
    group* p1GroupInd = pool[p1Index].groupPtr, * p2GroupInd = pool[p2Index].groupPtr;
    vec3 p1Center = p1GroupInd == NULL ? p1.translate : p1GroupInd->COM;
    vec3 p2Center = p2GroupInd == NULL ? p2.translate : p2GroupInd->COM;
    float p1Mass = p1GroupInd == NULL ? p1Phys.mass : p1GroupInd->totalMass;
    float p2Mass = p2GroupInd == NULL ? p2Phys.mass : p2GroupInd->totalMass;

    bool p1IsStationary = p1Phys.stationary == true;
    if(p1IsStationary)
    {
        p1Phys.linearV = vec3(0,0,0);
        p1Phys.angularV = vec3(0,0,0);
    }
    if(true)
    {
        //first treat p1 as point mass and calculate final velocity of p2, then do reverse.

        //the velocity that the point mass has is the vel of the object plus (the angular vel cross distance from COM to point mass)
        glm::vec3 p1PointVelocity = p1Phys.linearV + glm::cross(p1Phys.angularV, p1PointMass - p1Center); //true i think
        glm::vec3 p1pToP2 = p2Center - p1PointMass;
        // glm::vec3 p1PointAngularRelativeToP2 = (p1PointVelocity - (glm::normalize(p1pToP2) * glm::dot(glm::normalize(p1pToP2), p1PointVelocity)))/glm::length(p1pToP2);
        float p1PointAdjustedVelocity = glm::dot(normal, p1PointVelocity); //true
        float p2AdjustedVelocity = glm::dot(normal, p2Phys.linearV); //true
        glm::vec3 p1PointAdjustedAngularRelativeToP2 = 1.0f * ((normal*(p1PointAdjustedVelocity-p2AdjustedVelocity)) - (glm::normalize(p1pToP2) * glm::dot(glm::normalize(p1pToP2), (normal * p1PointAdjustedVelocity))))/glm::length(p1pToP2);
        
        //force can only be exerted by parts of velocity vectors parallel to normal vector (since no friction)
        glm::vec3 p2AdjustedAngularVelocity = p2Phys.angularV - (normal * glm::dot(normal, p2Phys.angularV));

        glm::vec3 p2LeftOverVelocity = p2Phys.linearV - (normal*p2AdjustedVelocity); //true
        glm::vec3 p2LeftOverAngularVelocity = p2Phys.angularV - p2AdjustedAngularVelocity;

        //now we find final linear and angular velocity for p2 separately.        
        //linear
        
        std::vector<float> finalLinearVelsP2 = finalVelocities(p1Mass, p2Mass, p1PointAdjustedVelocity, p2AdjustedVelocity, p1IsStationary, false, false);
        //angular
        glm::vec3 p2NewAngularVec = glm::cross(p1pToP2, normal);
        if(!v3Equal(p2NewAngularVec, vec3(0.0f,0.0f,0.0f)))
            p2NewAngularVec = glm::normalize(p2NewAngularVec);
        if(p1HasNormal)
            p2NewAngularVec *= -1;
        if(p1IsStationary)
        {
            // std::cout << "INITIAL: " << glm::length(p1PointAdjustedAngularRelativeToP2) << " " <<  glm::length(p2AdjustedAngularVelocity) << '\n';
        }
        std::vector<float> finalAngularVelsP2 = finalVelocities(p1Mass, p2Mass, glm::length(p1PointAdjustedAngularRelativeToP2), glm::length(p2AdjustedAngularVelocity), p1IsStationary, true, true);

        glm::vec3 p2linearFinal = normal * finalLinearVelsP2[1] + p2LeftOverVelocity;
        glm::vec3 p2angularFinal = p2NewAngularVec * finalAngularVelsP2[1] + p2LeftOverAngularVelocity;
        

        //------------------------------------------------------------------------------------------------------------
        
        glm::vec3 p2PointVelocity = p2Phys.linearV + glm::cross(p2Phys.angularV, p2PointMass - p2Center);
        glm::vec3 p2pToP1 = p1Center - p2PointMass;
        // glm::vec3 p2PointAngularRelativeToP1 = (p2PointVelocity - (glm::normalize(p2pToP1) * glm::dot(glm::normalize(p2pToP1), p2PointVelocity)))/glm::length(p2pToP1);
        float p2PointAdjustedVelocity = glm::dot(normal, p2PointVelocity);
        float p1AdjustedVelocity = glm::dot(normal, p1Phys.linearV);
        glm::vec3 p2PointAdjustedAngularRelativeToP1 = 1.0f * ((normal * (p2PointAdjustedVelocity-p1AdjustedVelocity)) - (glm::normalize(p2pToP1) * glm::dot(glm::normalize(p2pToP1), (normal * p2PointAdjustedVelocity))))/glm::length(p2pToP1);
        
        glm::vec3 p1AdjustedAngularVelocity = p1Phys.angularV - (normal * glm::dot(normal, p1Phys.angularV));

        glm::vec3 p1LeftOverVelocity = p1Phys.linearV - (normal * p1AdjustedVelocity);
        glm::vec3 p1LeftOverAngularVelocity = p1Phys.angularV - p1AdjustedAngularVelocity;
        //now we find final linear and angular velocity for p1 separately.        
        //linear
        
        std::vector<float> finalLinearVelsP1 = finalVelocities(p1Mass, p2Mass, p1AdjustedVelocity, p2PointAdjustedVelocity, p1IsStationary, false, false);
        //angular
        glm::vec3 p1NewAngularVec = glm::cross(p2pToP1, normal);
        if(!v3Equal(p1NewAngularVec, vec3(0.0f,0.0f,0.0f)))
            p1NewAngularVec = glm::normalize(p1NewAngularVec);
        if(!p1HasNormal)
            p1NewAngularVec *= -1;
        std::vector<float> finalAngularVelsP1 = finalVelocities(p1Mass, p2Mass, glm::length(p1AdjustedAngularVelocity), glm::length(p2PointAdjustedAngularRelativeToP1), p1IsStationary, true, false);

        glm::vec3 p1linearFinal = normal * finalLinearVelsP1[0] + p1LeftOverVelocity;
        glm::vec3 p1angularFinal = p1NewAngularVec * finalAngularVelsP1[0] + p1LeftOverAngularVelocity;

        glm::vec3 zeroVec{0.0f,0.0f,0.0f};
        

        //now we add velocity due to friction to both parts
        if(p1Index == 6)
        {
            p1LeftOverVelocity = vec3(0,0,0);
        }
        vec3 p1RelativeVelDirToP2 = p1LeftOverVelocity - p2LeftOverVelocity;
        if(!v3Equal(p1RelativeVelDirToP2, vec3(0,0,0)))
        {
            p1RelativeVelDirToP2 = glm::normalize(p1RelativeVelDirToP2);
        }
        // if(p1Index == 6)
        // {
        //     p1AdjustedVelocity = 0;
        // }
        float normalForce = p2Phys.mass * (finalLinearVelsP2[0] - p2AdjustedVelocity)/tickTime;
        // float normalForce = p1Phys.mass * (finalLinearVelsP1[0] - p1AdjustedVelocity)/tickTime;
        cols++;
        vec3 frictionForce =  p1RelativeVelDirToP2 * 0.5f * normalForce;
        vec3 impulse = frictionForce * tickTime;

        p2linearFinal += impulse/p2Phys.mass;
        vec3 p2Rad = p2PointMass - p2Center; float p2RSqrd = length(p2Rad) * length(p2Rad); 
        // p2angularFinal += 0.5f * glm::cross(p2Rad, impulse)/(p2RSqrd * p2Phys.mass);
        // std::cout << "leftOver\n";
        // printVec3(p2LeftOverVelocity);
        // std::cout << "add\n";
        // printVec3(impulse/p2Phys.mass);
        impulse *= -1;
        p1linearFinal += impulse/p1Phys.mass;
        vec3 p1Rad = p1PointMass - p1Center; float p1RSqrd = length(p1Rad) * length(p1Rad); 
        // p1angularFinal += 0.5f * glm::cross(p1Rad, impulse)/(p1RSqrd * p1Phys.mass);

        //setting final vels
        // std::cout << "start: "; printVec3()
        newVels[0] = p1linearFinal; newVels[1] = p1angularFinal; newVels[2] = p2linearFinal; newVels[3] = p2angularFinal;    }

}

//creates an explosion at position "pos". Causes parts nearby to detach from eachother and be pushed out.
void createExplosion(glm::vec3 pos, float radius, float strength)
{
    for(int i = 7; i < totalParts; i++)
    {
        PartData& currPart = allParts(i);
        PhysicsData& currPhys = allPhys(i);
        vec3 posToPart = currPart.translate - pos;
        float dist = glm::length(posToPart);
        if(dist <= radius)
        {
            //part is affected
            vec3 vel = glm::normalize(posToPart) * strength * (1 - dist/radius);
            group* g = pool[i].groupPtr;
            if(g != NULL)
            {
                g->linearV += vel/g->totalMass;
                if(glm::length(vel) > 0.02f)
                    detachPart(i);
            }
            else
            {
                currPhys.linearV += vel;
            }
        }
    }
    // for(group& g : groups)
    // {
    //     vec3 posToGroup = g.COM - pos;
    //     float dist = glm::length(posToGroup);
    //     if(dist <= radius)
    //     {
    //         //part is affected
    //         vec3 vel = glm::normalize(posToGroup) * strength * (1 - dist/radius);
    //         g.linearV += vel;
    //     }
    // }
}


//applies the angular and linear velocity of a group to all parts connected and in that group
void updateGroup(group& currGroup)
{
    for(int j = 0; j < currGroup.members.size(); j++)
    {
        PhysicsData& currPhys = allPhys(currGroup.members[j]);
        currPhys.linearV = currGroup.linearV;
        currPhys.angularV = currGroup.angularV; 
    }
}

//applies a downward force to all parts
void applyGravity()
{
    //first update group part vels and group center of masses
    auto it = groups.begin();
    for(; it != groups.end(); it++)
    {
        group& currGroup = *it;
        currGroup.linearV += glm::vec3(0.0f,-0.001f,0.0f);
        updateGroup(currGroup);
    }

    //now update other parts vels and move all parts
    for(int i = 6; i < totalParts; i++)
    {
        PartData& currPart = allParts(i);
        PhysicsData& currPhys = allPhys(i);
        
        if(pool[i].groupPtr == NULL && currPhys.stationary == false)
        {
            currPhys.linearV += glm::vec3(0.0f,-0.001f,0.0f);
        }
        
        currPhys.linearV += currPhys.linearA;
        currPhys.angularV += currPhys.angularA;
    }
}

//move parts by their linear velocity, and rotate parts by their angular velocity
// void moveParts()
// {
//     //first update group parts vels and group center of masses
//     auto it = groups.begin();
//     for(; it != groups.end(); it++)
//     {
//         group& currGroup = *it;
//         currGroup.linearV -= currGroup.linearV/200.0f;
//         currGroup.COM += currGroup.linearV;
//         updateGroup(currGroup);
//     }

//     //now update other parts vels and move all parts
//     for(int i = 6; i < totalParts; i++)
//     {
//         PartData& currPart = allParts(i);
//         PhysicsData& currPhys = allPhys(i);
        
//         if(currPhys.angularV != glm::vec3(0,0,0))
//         {
//             currPart.rotate(glm::normalize(currPhys.angularV), glm::length(currPhys.angularV));
//             //now we need to do additional translating if the part is in a group, which depends on how far away the part is from the groups center of mass.
//             if(pool[i].groupPtr != NULL)
//             {
//                 group& currGroup = *(pool[i].groupPtr);
//                 glm::vec3 COMToObj = currPart.translate - currGroup.COM;
//                 //rotate object around COM of group
//                 glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::length(currPhys.angularV), glm::normalize(currPhys.angularV));
//                 glm::vec3 newCOMToObj = glm::vec3(rotMat * glm::vec4(COMToObj, 1.0f));
//                 currPart.translate = currGroup.COM + newCOMToObj;
//             }
//         }
//         if(pool[i].groupPtr == NULL && currPhys.stationary == false)
//         {
//             currPhys.linearV -= currPhys.linearV/200.0f;
//         }
//         currPart.translate += currPhys.linearV;
//     }
// }


//for each part, update its velocity, then its position based on this new velocity
void updateVelsMoveParts()
{
    //first update group part vels and group center of masses
    auto it = groups.begin();
    for(; it != groups.end(); it++)
    {
        group& currGroup = *it;
        currGroup.linearV += glm::vec3(0.0f,-0.002f,0.0f);
        currGroup.linearV -= currGroup.linearV/200.0f;
        currGroup.angularV -= currGroup.angularV/200.0f;
        currGroup.COM += currGroup.linearV;
        updateGroup(currGroup);
    }

    //now update other parts vels and move all parts
    for(int i = 6; i < totalParts; i++)
    {
        PartData& currPart = allParts(i);
        PhysicsData& currPhys = allPhys(i);
        
        if(currPhys.angularV != glm::vec3(0,0,0))
        {
            currPart.rotate(glm::normalize(currPhys.angularV), glm::length(currPhys.angularV));
            //now we need to do additional translating if the part is in a group, which depends on how far away the part is from the groups center of mass.
            if(pool[i].groupPtr != NULL)
            {
                group& currGroup = *(pool[i].groupPtr);
                glm::vec3 COMToObj = currPart.translate - currGroup.COM;
                //rotate object around COM of group
                glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::length(currPhys.angularV), glm::normalize(currPhys.angularV));
                glm::vec3 newCOMToObj = glm::vec3(rotMat * glm::vec4(COMToObj, 1.0f));
                currPart.translate = currGroup.COM + newCOMToObj;
            }
        }
        if(pool[i].groupPtr == NULL && currPhys.stationary == false)
        {
            currPhys.linearV += glm::vec3(0.0f,-0.002f,0.0f);
            currPhys.linearV -= currPhys.linearV/200.0f;
            currPhys.angularV -= currPhys.angularV/200.0f;
        }
        
        currPhys.linearV += currPhys.linearA;
        currPhys.angularV += currPhys.angularA;
        currPart.translate += currPhys.linearV;
    }
}
int skip = 0;
int numTicks = 0;
bool timingNow = false;
//this function is called every frame to compute the new accelerations, velocities, and position of every part.
void physicsTick()
{
    if(startTiming)
    {
        numTicks = 0;
        start1 = high_resolution_clock::now();
        startTiming = false;
        timingNow = true;
    }
    numTicks++;
    if(numTicks == 60 && timingNow == true)
    {
        end1 = high_resolution_clock::now();
        numTicks = 0;
        auto duration = duration_cast<milliseconds>(end1 - start1);
        std::cout << duration.count() << '\n';
        timingNow = false;
    }
    // std::cout << "GROUP ARE:\n";
    // for(const auto& currGroup: groups)
    // {
    //     for(int i = 0; i < currGroup.members.size(); i++)
    //     {
    //         std::cout << currGroup.members[i] << '\n';
    //     }
    //     std::cout << "next\n";
    // }
    updateVelsMoveParts();
    // applyGravity();
    // skip++;
    // if(skip != 2)
    // {
    //     return;
    // }
    // else
    // {
    //     skip = 0;
    // }
    // auto start = high_resolution_clock::now();
    for(int i = 6; i < totalParts; i++)
    {
        PartData& part1 = allParts(i);
        PhysicsData& physics1 = allPhys(i);
        
        unsigned int p1Type = pool[i].type;
        group* p1GroupInd = pool[i].groupPtr;
        
        //check for collision with every other part
        float p1Max = std::max(std::max(part1.scaleVector.x, part1.scaleVector.y), part1.scaleVector.z);
        for(int j = i+1; j < totalParts; j++)
        {
            if(i == 6)
            {
                physics1.linearV = vec3(0,0,0);
                physics1.angularV = vec3(0,0,0);
            }
            // continue;
            PartData& part2 = allParts(j);
            PhysicsData& physics2 = allPhys(j);
            unsigned int p2Type = pool[j].type;
            group* p2GroupInd = pool[j].groupPtr;

            //if they are part of the same group, don't check for intersection
            if(p1GroupInd == p2GroupInd && (p1GroupInd != NULL && p2GroupInd != NULL))
            {
                // std::cout << i << j << '\n';
                continue;
            }
            float p2Max = std::max(std::max(part2.scaleVector.x, part2.scaleVector.y), part2.scaleVector.z);
            if(glm::length(part1.translate - part2.translate) > (p2Max + p1Max + 2.0f))
            {
                // std::cout << i << j << '\n';
                //it is not possible for p1 and p2 to intersect
                continue;
            }
            vec3 p2ToP1 = part1.translate - part2.translate;

            vec3 p1MaxInDir = maxPtInDir(i, -p2ToP1);
            vec3 p2MaxInDir = maxPtInDir(j, p2ToP1);
            if(glm::dot(p2MaxInDir, p2ToP1) < glm::dot(p1MaxInDir, p2ToP1))
            {
                // std::cout << i << j << "cut\n";
                continue;
            }

            // std::cout << "STILL GOING " << i << j << '\n';
            //if we have reached this point, we check for intersection
            glm::vec3 intPts[3];
            bool result = SAT(i, j, intPts);
            //if they don't collide, continue
            if(!result)
                continue;

            glm::vec3 newVels[4];
            partsCollide(i,j,intPts[1], intPts[0], intPts[2], newVels);
            glm::vec3 p1IntPt, p2IntPt;
            // std::cout << "start: "; printVec3(physics1.linearV)
            physics1.linearV = newVels[0];
            physics1.angularV = newVels[1];
            physics2.linearV = newVels[2];
            physics2.angularV = newVels[3];
            // if(isnan(physics1.linearV.x))
            // {
            //     std::cout << i << " IS NAN IND\n";
            // }
            // if(isnan(physics2.linearV.x))
            // {
            //     std::cout << j << " IS NAN IND\n";
            // }
            // if(isnan(part1.translate.x))
            // {
            //     std::cout << i << " TRANSTRANS IS NAN IND\n";
            // }
            // if(isnan(part2.translate.x))
            // {
            //     std::cout << j << " TRANSTRANS IS NAN IND\n";
            // }
            if(p1GroupInd != NULL)
            {
                group& p1Group = *p1GroupInd;
                p1Group.angularV = physics1.angularV; p1Group.linearV = physics1.linearV;
                updateGroup(p1Group);
            }
            if(p2GroupInd != NULL)
            {
                group& p2Group = *p2GroupInd;
                p2Group.angularV = physics2.angularV; p2Group.linearV = physics2.linearV;
                updateGroup(p2Group);
            }
            int a= 1;
        }
        //set ground part vels to 0
        if(physics1.stationary == true)
        {
            physics1.linearV = glm::vec3(0.0f,0.0f,0.0f);
            physics1.angularV = glm::vec3(0.0f,0.0f,0.0f);
        }
    }
    // moveParts();
    
    
}

//set the velocities of all parts to 0
void setVelsZero()
{
    for(int i = 0; i < totalParts; i++)
    {
        PhysicsData& currPhys = allPhys(i);
        currPhys.linearV = glm::vec3(0.0f,0.0f,0.0f);
        currPhys.angularV = glm::vec3(0.0f,0.0f,0.0f);
        PartData& currPart = allParts(i);
        currPart.rotationMatrix = glm::mat4(1.0f);
        currPart.normalMatrix = glm::mat3(1.0f);
    }
}

//THIS FUNCTION IS NOT USED
//this function checks if an edge intersects a cylinder
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
        // std::cout << "no intersection\n";
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
        // std::cout << "INTERSECTION\n";
    }
    else
    {
        // std::cout << "NO INTERSECTION (end)\n";
    }
    // std::cout << "CLOSEST POINT IS " << closestPoint.x << ", " << closestPoint.y << ", " << closestPoint.z << '\n';
    // std::cout << "botCyl_2D is " << botCyl_2D.x << ", " << botCyl_2D.y << ", " << botCyl_2D.z << '\n';
}
