#include <iostream>
#define GLEW_STATIC 1
#include "include/glew.h"
#include "include/ShaderProgram.h"
#include "include/LoadTexture2D.h"
#include "../glfw-3.3.7.bin.WIN64/include/GLFW/glfw3.h"
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#include "../glm/glm/gtx/intersect.hpp"
#include "include/VAOData.h"
#include "include/ShaderSources.h"
#include "include/PhysicsData.h"
#include "include/DrawPrims.h"
#include "include/building.h"
#include "include/game.h"
#include "include/saveload.h"
#include <filesystem>
#include <fstream>
#include <utility>

using namespace glm; 

namespace fs = std::filesystem;
//Puts information about all of the parts (position, color, etc) into a save file which can be loaded later
void saveWorld(std::string name)
{
    //check if it already exists, if so, overwrite, otherwise, make a new file
    std::string newFileName = name + ".txt";
    std::string path = ".\\worlds";

    //write to filenow

    //write any vector data all in a row

    //poolinfo, then part data, then groups
    bool test = false;
    std::fstream file(path + "\\" + newFileName, std::fstream::out | std::fstream::binary);
    file.exceptions(std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit);
    if(!test)
    {
        //we write number of entries in poolinfo vector so we know how many to read later
        file.write((char*)&totalParts, sizeof(totalParts));

        //now write partinfo
        file.write((char*)(pool.data()), sizeof(partInfo) * totalParts);
        //now write everything that was in adjacency lists
        for(int i = 0; i < totalParts; i++)
        {
            std::vector<int>& currPI = pool[i].nodeInfo.adjacent;
            int currSize = currPI.size();
            file.write((char*)&currSize, sizeof(int));
            file.write((char*)currPI.data(), sizeof(int) * currSize);
        }
        //now write partdata + physdata  for everything (along with how many parts are of each type)
        for(int i = 0; i < numVAOs; i++)
        {
            VAOData& currVAO = *VAOs[i];
            file.write((char*)&currVAO.numParts, sizeof(currVAO.numParts));
            file.write((char*)(currVAO.parts.data()), sizeof(PartData) * currVAO.numParts);
            file.write((char*)(currVAO.partsPhysics.data()), sizeof(PhysicsData) * currVAO.numParts);
        }
        // now write groups along with group members
        int numGroups = groups.size();
        file.write((char*)&numGroups, sizeof(numGroups));
        int i = 0;
        for(group& g : groups)
        {
            file.write((char*)&g, sizeof(group));
            int currSize = g.members.size();
            file.write((char*)&currSize, sizeof(currSize));
            file.write((char*)g.members.data(), sizeof(int) * currSize);
            i++;
        }
        file.close();
    }
    return;
}


//deletes a world and all traces of it
void deleteWorld(std::string name)
{
    //remove the world from the saves lists
    auto it = std::find(saves.begin(), saves.end(), name);
    if(it == saves.end())
    {
        std::cout << "ERROR: Trying to delete world that doesn't exist";
        return;
    }
    saves.erase(it);
    
    //remove the world save button
    for(int i = 0; i < nameButtons.size(); i++)
    {
        if(nameButtons[i].text == name)
            nameButtons.erase(nameButtons.begin() + i);
    }
    //now we find the file in the filesystem and delete it
    fs::remove(".\\words\\" + name + ".txt");
    std::cout << "removed " << name << " save.\n";
}

//populate buttons vector
void newSave(std::string name)
{
    nameButtons.push_back({vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &nameButtonClicked, true, name, glm::vec3(1.0,1.0f,1.0f)});
    saves.push_back(name);
}

void getOldSaves()
{
    for (const auto& entry : fs::directory_iterator(".\\worlds")) {
        std::string filename = entry.path().filename().string();
        size_t lastIndex = filename.find_last_of("."); 
        std::string nameNoExtension = filename.substr(0, lastIndex); 
        newSave(nameNoExtension);
    }
}

extern ShaderProgram cubeProgram;
std::vector<int> emptyVec;
//Loads the parts from a world into the current scene
void loadWorld(std::string name)
{
    selectedParts.clear();
    std::string newFileName = name + ".txt";
    std::string path = ".\\worlds\\" + newFileName;
    if(!fs::exists(path))
    {
        std::cout << "ERROR: File doesn't exist. Can't read it.\n";
        std::cout << path << '\n';
        return;
    }
    std::fstream file(path, std::fstream::in | std::fstream::binary);

    //get number of parts and all partinfo
    file.read((char*)&totalParts, sizeof(int));
    pool.resize(totalParts);
    file.read((char*)pool.data(), sizeof(partInfo) * totalParts);

    //read into adjacency lists
    for(int i = 0; i < totalParts; i++)
    {
        std::vector<int>& currAdj = pool[i].nodeInfo.adjacent;
        int currSize;
        //get size of list
        file.read((char*)&currSize, sizeof(currSize));
        std::vector<int> victim(currSize);
        //fill list
        file.read((char*)victim.data(), sizeof(int) * currSize);
        //cannibalize victim's data
        std::swap(currAdj, victim);
        memcpy(&victim, &emptyVec, sizeof(emptyVec));
    }
    //read into part info for each part type
    for(int i = 0; i < numVAOs; i++)
    {
        VAOData& currVAO = *VAOs[i];
        currVAO.numParts = 0;
        currVAO.parts.clear();
        currVAO.partsPhysics.clear();
        if(!file.read((char*)&currVAO.numParts, sizeof(currVAO.numParts)))
        {
            std::cout << "ERROR11\n";
        }
        currVAO.parts.resize(currVAO.numParts);
        currVAO.partsPhysics.resize(currVAO.numParts);
        file.read((char*)currVAO.parts.data(), sizeof(PartData) * currVAO.numParts);
        glBindBuffer(GL_ARRAY_BUFFER, currVAO.partsBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(PartData) * currVAO.parts.capacity(), (void*)(currVAO.parts.data()), GL_DYNAMIC_DRAW);
        file.read((char*)currVAO.partsPhysics.data(), sizeof(PhysicsData) * currVAO.numParts);
    }
    //read into groups
    int numGroups = 0;
    file.read((char*)&numGroups, sizeof(numGroups));
    groups.clear();
    groups.resize(numGroups);
    for(group& g : groups)
    {
        file.read((char*)&g, sizeof(group));
        int numMembers;
        file.read((char*)&numMembers, sizeof(numMembers));
        
        std::vector<int> victim(numMembers);
        file.read((char*)victim.data(), sizeof(int) * numMembers);
        std::swap(g.members, victim);
        memcpy(&victim, &emptyVec, sizeof(emptyVec));
        //now we have to update the group ptr in every member
        for(int i = 0; i < g.members.size(); i++)
        {
            pool[g.members[i]].groupPtr = &g;
        }
    }
    file.close();
}