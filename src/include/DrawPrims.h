#pragma once
#define GLEW_STATIC 1
#include <string>
#include "include/glew.h"
#include "LoadTexture2D.h"
#include "../glfw-3.3.7.bin.WIN64/include/GLFW/glfw3.h"
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#include "../glm/glm/gtx/intersect.hpp"

struct buttonData
{
    glm::vec2 scale;
    glm::vec2 position;
    glm::vec3 color;
    bool visible;
    texture tex;
    void (*onClick)(buttonData& btn);
    bool useText = false;
    std::string text;
    glm::vec3 textColor;
};

typedef buttonData rectangleData;

extern std::vector<buttonData> buttons;
extern void drawPoint(glm::vec3 screenPos, glm::vec3 color);
extern void drawButton(buttonData& button);
extern void drawSquareFrame(glm::vec3* pts);
extern void drawSceneButtons();
extern void initButtons();
extern void drawRectangle(rectangleData& rectangle);
extern void setScalePartsPosAndScale(std::vector<int>& currSelected);
extern void drawScaleParts();
extern rectangleData saveMenuBox;
extern void drawSaveMenu();
extern void drawTitleScreen();
extern void drawInstructionMenu();
extern void createScaleParts();
extern void updateScaleParts();
extern void scalePartsDisappear();
extern glm::vec3 scalePartsPositions[6];
extern glm::vec3 scalePartsScales[6];
extern glm::vec3 circlePositions[6];

extern float menuTop;
extern float menuLeft;
extern float menuRight;
extern float menuBottom;
extern glm::mat4 saveProj;
extern glm::vec2 getViewportDims();
extern std::vector<std::string> saves;

extern void nameButtonClicked(buttonData& btn);

extern std::vector<buttonData> nameButtons;
extern buttonData colorButton;
extern bool tbClicked;
extern bool creatingTB;