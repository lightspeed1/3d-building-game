#pragma once
#include <string>

extern void saveWorld(std::string name);
extern void loadWorld(std::string name);
extern void deleteWorld(std::string name);
extern void newSave(std::string name);
extern void getOldSaves();