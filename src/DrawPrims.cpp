#include <iostream>
#include <vector>
#include "include/VAOData.h"
#include "include/PhysicsData.h"
#include <algorithm>
#include "include/DrawPrims.h"
#include "include/game.h"
#include "include/LoadTexture2D.h"
#include "include/building.h"
#include "include/text.h"
#include "include/saveload.h"
using namespace std;
using namespace glm;

//globals for menu boxes
rectangleData saveMenuBox{glm::vec2(2.0f,2.0f), glm::vec2(0.0f,0.0f), glm::vec3(0.8f,0.8f,0.8f), true};
rectangleData saveMenuHeaderBox{glm::vec2(), glm::vec2(), glm::vec3(0.8f,0.8f,0.8f), true};
rectangleData titleScreenBox{glm::vec2(2.0f,2.0f), glm::vec2(0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), true};
//globals for menu dimension
float menuTop = ((saveMenuBox.position.y + (0.5f * saveMenuBox.scale.y)) * pixelHeight/2.0f) + pixelHeight/2.0f;
float menuLeft = ((saveMenuBox.position.x + (-0.5f * saveMenuBox.scale.x)) * pixelWidth/2.0f) + pixelWidth/2.0f;
float menuRight = pixelWidth - menuLeft;
float menuBottom = pixelHeight - menuTop;

glm::mat4 saveProj = glm::ortho(menuLeft, menuRight, menuBottom, menuTop);


//shader sources for rendering everything except regular parts, which includes points, square frames, scale parts, and rectangles.
const char* pointVertShaderSource = R"(
#version 460 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 pointColor;

uniform vec3 vPos;
uniform vec3 myColor;
uniform mat4 view;
uniform mat4 projection;

out vec3 color_;

void main()
{
    color_ = pointColor;
    vec4 it = projection * view * vec4(vertexPos, 1.0f);
    gl_Position = vec4(it.x, it.y, it.z, it.w);
    // gl_Position = vec4(vertexPos, 1.0f);
    gl_PointSize = 10.0f;
}
)";
const char* pointFragShaderSource = R"(
#version 460 core
in vec3 color_;
out vec4 FragColor;

void main()
{
    FragColor = vec4(color_, 1.0f);
}
)";

const char* squareFrameVertShader = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
void main()
{
    gl_Position = vec4(vertexPos, 1.0f);
}
)";
const char* squareFrameFragShader = R"(
#version 460 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f,0.0f,0.0f, 1.0f);
}
)";

const char* buttonVertShader = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 texCoord;
uniform vec2 scale;
uniform vec2 position;
uniform vec3 color;
out vec3 color_;
out vec2 texCoord_;
// uniform mat4 projection;
void main()
{
    gl_Position = vec4((vec3(scale,1.0f) * vertexPos) + vec3(position, 0.0f), 1.0f);
    color_ = color;
    texCoord_ = texCoord;
}
)";
const char* buttonFragShader = R"(
#version 460 core
out vec4 FragColor;
in vec3 color_;
in vec2 texCoord_;
uniform vec3 texMultiplier;
uniform sampler2D myTexture;
void main()
{
    vec3 col = (texMultiplier * vec3(texture(myTexture, texCoord_))) + ((vec3(1.0f,1.0f,1.0f) - texMultiplier) * color_);
    FragColor = vec4(col, 1.0f);
}
)";

const char* rectangleVertShader = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 texCoord;
uniform vec2 scale;
uniform vec2 position;
uniform vec3 color;
out vec3 color_;
// uniform mat4 projection;
void main()
{
    gl_Position = vec4((vec3(scale,1.0f) * vertexPos) + vec3(position, 0.0f), 1.0f);
    color_ = color;
}
)";
const char* rectangleFragShader = R"(
#version 460 core
out vec4 FragColor;
in vec3 color_;
void main()
{
    FragColor = vec4(color_, 1.0f);
}
)";

const char* scalePartsVertShader = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
// layout (location = 1) in vec2 texCoord;
uniform vec3 translate;
uniform vec3 scale;
uniform mat4 view;
uniform mat4 projection;
// out vec2 texCoord_;

void main()
{
    // vec4 pos1 = view * vec4(translate, 1.0f);
    // float s = 0.3 * pos1.z;
    // vec3 scale = s * vec3(0.1, 0.1, 0.1);
    vec4 pos = projection * view * vec4(translate + (scale * vertexPos), 1.0f);
    
    gl_Position = pos;
    // texCoord_ = texCoord;
}
)";
const char* scalePartsFragShader = R"(
#version 460 core
// in vec2 texCoord_;
out vec4 FragColor;
// uniform sampler2D circleTexture;
void main()
{
    vec4 color = vec4(0.5f, 1.0f, 0.5f, 0.5f);
    FragColor = color;
}
)";

//IDs for VBOs and VAOs, and shader programs of primitives. 
unsigned int pointID = NULL;
unsigned int pointVAO = NULL;
ShaderProgram pointProgram(pointVertShaderSource, pointFragShaderSource);

unsigned int squareFrameID = NULL;
unsigned int squareFrameVAO = NULL;
ShaderProgram squareFrameProgram(squareFrameVertShader, squareFrameFragShader);

unsigned int buttonID = NULL;
unsigned int buttonVAO = NULL;
ShaderProgram buttonProgram(buttonVertShader, buttonFragShader);

unsigned int scalePartsVAO = NULL;
ShaderProgram scalePartsProgram(scalePartsVertShader, scalePartsFragShader);

unsigned int rectangleID = NULL;
unsigned int rectangleVAO = NULL;
ShaderProgram rectangleProgram(rectangleVertShader, rectangleFragShader);

std::string currFile = "";

glm::vec3 circlePositions[6];

std::vector<buttonData> buttons;
std::vector<buttonData> nameButtons;
//button VB Data
static GLfloat buttonPts[] = 
{
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
};

std::vector<texture> partButtonTextures{
    {"images\\cube.png"},
    {"images\\slant.png"},
    {"images\\cylinder.png"}
};

std::vector<std::string> saves;

//adjusts the size of the scale parts so that they take up the same amount of the screen no matter how for away the user is away from thm.
//also, for each scale part, set their position to be 1 meter away on the positive or negative x, y, or z axis from the selecte parts' furthest point on that axis.
void setScalePartsPosAndScale(std::vector<int>& currSelected)
{
    for(int i = 0; i < 6; i++)
    {
        PartData& currScalePart = allParts(i);

        vec3 currNorm = VAOs[GAMECUBE]->normalsArray[i];
        if(currSelected.size() == 1)
        {
            PartData& part = allParts(currSelected[0]);
            currNorm = part.applyNM(VAOs[GAMECUBE]->normalsArray[i]);
        }
        vec3 maxPt = maxPtInDirMult(currSelected, currNorm);
        vec3 mid = centerOfParts(currSelected);
        vec3 scalePos = currNorm + (currNorm * glm::dot(currNorm, maxPt - mid)) + mid;
        // scalePartsPositions[i] = scalePos;
        currScalePart.translate = scalePos;
        vec3 v = scalePos;
        vec4 pos1 = view * vec4(v.x, v.y, v.z, 1.0f);
        float s1 = 0.5 * pos1.z;
        vec3 s = s1 * vec3(0.1, 0.1, 0.1);
        currScalePart.scale(s);
    }
}

//initializes the 6 scale parts
void createScaleParts()
{
    for(int i = 0; i < 6; i++)
    {
        PartData& currPart = *(VAOs[GAMECUBE]->addPart());
        currPart.translate = glm::vec3(0.0f, -100.0f,0.0f);
    }
}

//puts the scale parts in the bottom of the scene so the user can't see them.
void scalePartsDisappear()
{
    for(int i = 0; i < 6; i++)
        allParts(i).translate = glm::vec3(0.0f, -200.0f,0.0f);
}

void updateScaleParts()
{
    if(selectedParts.size() <= 0)
        return;
    setScalePartsPosAndScale(selectedParts);
}

bool tbClicked = false;


//the functions below show what happens when each button on screen is clicked.
void unfreezeButtonClicked(buttonData& btn)
{
    gameMode = UNFREEZE;
    std::cout << "UNFREEZE\n";
    buttons[1].visible = true;
    buttons[0].visible = false;
}

void freezeButtonClicked(buttonData& btn)
{
    gameMode = FREEZE;
    setVelsZero();
    std::cout << "FREEZE\n";
    buttons[1].visible = false;
    buttons[0].visible = true;
}


void saveButtonClicked(buttonData& btn)
{
    saveWorld(currFile);
}

void deleteSaveButtonClicked(buttonData& btn)
{
    deleteWorld(currFile);
}

void nothingFunc(buttonData& btn)
{
    return;
}

void nameButtonClicked(buttonData& btn)
{
    // if(!creatingTB)
    // {
        currFile = btn.text;
        tbClicked = true;
    // }
    // else
    // {
    //     creatingTB = false;
    //     currTextBox = NULL;
        
    // }
}

bool creatingTB = false;
void newSaveButtonClicked(buttonData& btn)
{
    newSave("");
    currTextBox = &nameButtons.back();
    creatingTB = true;
}

//changes the type of new part that will be added
void partButtonClicked(buttonData& btn)
{
    currNewPart = (currNewPart + 1)%numVAOs;
    std::cout << "CURRNEW PART IS " << currNewPart << '\n';
    //we must initialize texture
    buttons[2].tex = partButtonTextures[currNewPart];
    
    if(buttons[2].tex.textureUnit == -1)
    {
        buttons[2].tex.textureID = loadTexture2D(buttons[2].tex.texturePath.c_str(), &(buttons[2].tex.textureUnit));
        partButtonTextures[currNewPart] = buttons[2].tex;
    }
}

void loadButtonPressed(buttonData& btn)
{
    loadWorld(currFile);
}

void colorButtonClicked(buttonData& btn)
{
    if(selectedParts.size() > 0)
    {
        currTextBox = &btn; colorTB = true;
    }
    tbClicked = true;
}

void playButtonClicked(buttonData& btn)
{
    gameState = PLAYING;
    gameMode = FREEZE;
}

void instructionsButtonClicked(buttonData& btn)
{
    gameState = INSTRUCTIONS;
}

void backButtonClicked(buttonData& btn)
{
    gameState = TITLESCREEN;
}

buttonData colorButton{vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &colorButtonClicked, true, "COLOR", vec3(1.0f,1.0f,1.0f)};

//create the objects for the buttons and the textures associated with them.
void initButtons()
{
    buttons = std::vector<buttonData>{
        {vec2(100.0f,100.0f), vec2(), vec3(1.0f,0.0f,0.0f), true, {"images\\unfreeze.png"}, &unfreezeButtonClicked, false}, // freeze
        {vec2(100.0f,100.0f), vec2(), vec3(1.0f,0.0f,0.0f), true, {"images\\freeze.png"}, &freezeButtonClicked, false}, // unfreeze
        {vec2(100.0f,100.0f), vec2(), vec3(1.0f,0.0f,0.0f), true, {"images\\cube.png"}, &partButtonClicked, false},
        {vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {"images\\trash.png"}, &deleteSaveButtonClicked, false}, // delete save button
        {vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &saveButtonClicked, true, "SAVE", vec3(1.0,1.0f,1.0f)},
        {vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &loadButtonPressed, true, "LOAD", vec3(1.0,1.0f,1.0f)},
        {vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &newSaveButtonClicked, true, "NEW SAVE", vec3(1.0,1.0f,1.0f)},
        {vec3(), vec3(), vec3(0.0f,0.0f,0.0f), true, {}, &nothingFunc, true, "MY GAME", vec3(1.0f,1.0f,1.0f)},
        {vec3(), vec3(), vec3(0.5f,0.4f,0.4f), true, {}, &playButtonClicked, true, "PLAY", vec3(1.0f,0.9f,0.9f)},
        {vec3(), vec3(), vec3(0.4f,0.5f,0.5f), true, {}, &instructionsButtonClicked, true, "INSTRUCTIONS", vec3(0.9f,1.0f,1.0f)},
        {vec3(), vec3(0.0f,0.0f,0.0f), vec3(0.2f,0.2f,0.2f), true, {}, &backButtonClicked, true, "BACK", vec3(1.0f,1.0f,1.0f)}
    };
    for(int i = 0; i < buttons.size(); i++)
    {
        buttonData& currData = buttons[i];
        if(currData.tex.texturePath == "")
            continue;
        currData.tex.textureID = loadTexture2D(currData.tex.texturePath.c_str(), &(currData.tex.textureUnit));
    }
    partButtonTextures[GAMECUBE] = buttons[2].tex;
    

}

//translates the pixel widht and height of a button to NDC coordinates representing it's size.
glm::vec2 buttonScale(float xPixels, float yPixels)
{
    return((screenCoordsToNDC(xPixels/2.0f, yPixels/2.0f) + glm::vec2(1.0f,1.0f))/0.5f);
}

glm::vec2 buttonPosition(vec2 buttonSize, float xPixels, float yPixels)
{
    float buttonWidth = 2*(buttonSize.x * 0.5f) * pixelWidth/2.0f;
    float buttonHeight = 2*(buttonSize.y * 0.5f) * pixelHeight/2.0f;
    float buttonPosXPix = buttonWidth/2.0f + xPixels;
    float buttonPosYPix = buttonHeight/2.0f + yPixels;

    glm::vec2 ndcCoords = screenCoordsToNDC(buttonPosXPix, buttonPosYPix);
    return ndcCoords;
}

//render the buttons that should be displayed while in PLAYING mode
void drawSceneButtons()
{
    for(int i = 0; i < 3; i++)
    {
        buttonData& currData = buttons[i];
        if(currData.visible)
        {
            currData.position = vec2(pixelWidth - ((i+1) * 100.0f), 0.0f);
            drawButton(currData);
        }

        //now draw the color text box
        textInfo strInfo = getTextInfo(colorButton.text, 1.0f);
        float buttonDefaultLen = getTextInfo("COLOR", 1.0f).totalWidth/2.0f;
        glm::vec2 buttonSize = vec2(fmaxf(strInfo.totalWidth,buttonDefaultLen), 80.0f);
        colorButton.scale = buttonSize; colorButton.position = vec2(0.0f,0.0f);
        drawButton(colorButton);
        
        //draw 
    }
}


//render the buttons that should be displayed while in TITLESCREEN mode
void drawTitleScreen()
{
    drawRectangle(titleScreenBox);
    //render title, play button, and instructions button
    buttonData& title = buttons[7];
    buttonData& playBtn = buttons[8];
    buttonData& instrsBtn = buttons[9];

    textInfo titleInfo = getTextInfo(title.text, 1.0f);
    title.scale = vec2(titleInfo.totalWidth, titleInfo.maxTop - titleInfo.minBottom);
    title.position = vec2(pixelWidth/2.0f - title.scale.x/2.0f, pixelHeight - title.scale.y);
    drawButton(title);

    textInfo playInfo = getTextInfo(playBtn.text, 1.0f);
    playBtn.scale = vec2(playInfo.totalWidth, playInfo.maxTop - playInfo.minBottom);
    playBtn.position = vec2(pixelWidth/2.0f - playBtn.scale.x/2.0f, pixelHeight * 0.75 - playBtn.scale.y/2.0f);
    drawButton(playBtn);

    textInfo instrsInfo = getTextInfo(instrsBtn.text, 1.0f);
    instrsBtn.scale = vec2(instrsInfo.totalWidth, instrsInfo.maxTop - instrsInfo.minBottom);
    instrsBtn.position = vec2(pixelWidth/2.0f - instrsBtn.scale.x/2.0f, pixelHeight * 0.25 - instrsBtn.scale.y/2.0f);
    drawButton(instrsBtn);

}

//instructions:
const char* instructionsText = 
R"(
OVERVIEW: 
-This is a building game. You can build with three kinds of parts; cubes, slants, 
and cylinders. 

--MOVING AROUND: 
-You can move the camera around by pressing W, A, S, and D. 
Change the rotation of the camera by holding the right mouse button down and dragging the mouse. 

--MANIPULATING PARTS: 
-Select a part by clicking on it. Select multiple parts by holding ctrl, then clicking and dragging. 
Create a new part by selecting the part you want to build it on and pressing F. You can change the color of a part by clicking on it, 
selecting the "COLOR" button and typing an RGB value (three numbers). Each number in this value ranges from 0.0 to 1.0 
and is separated from the others with a comma. A part can be moved by clicking and 
dragging it around. When a part is selected, press q to toggle between scaling and moving parts. When the cubes surrounding the selected parts are blue, 
the part can be scaled up or down by dragging these surrounding cubes. When they are yellowish brown, drag them to move the selected parts. 
Delete a part by selecting it and pressing BACKSPACE. 

--SAVING WORLDS: 
-You can save a world you have created by pressing L to bring up the save menu. Within the 
menu, you will see all of your saved worlds. You can create a new save by clicking the 
button at the bottom and then typing the name of the save, followed by ENTER. To write to 
a world, click its name and click SAVE. You can load a world into the current scene with 
the LOAD button, and delete it by pressing the trash can button next to the save. 

--FREEZE AND UNFREEZE: 
-While playing the game, you can hold the parts in place by clicking the FREEZE button. 
Clicking the UNFREEZE button will cause all parts to be affected by physics. They will fall 
due to gravity and collide with eachother. Make sure you save your world before you do this!

--EXPLOSIONS: 
-You can cause parts to detach from eachother by pressing Z when the parts are unfrozen. 
When Z is pressed, an explosion will be created where your mouse is pointing, causing 
parts to detach from one another and be blown away.
)";

//inserts newline characters in a string so that it doesn't go off the screen when displayed.
std::string formatText(std::string s)
{
    int pos = 0;
    int currWordLen = 0;
    int lastSpaceIndex = 0;
    int spaceLen = characters[' '].advance >> 6;
    for(int i = 0; i < s.size(); i++)
    {
        bool lastChar = (i == s.size() - 1);
        //if the current character is space, or we are on the last character
        if(s[i] == ' ' || (currWordLen != 0 && lastChar))
        {
            //check if the current word goes past the right edge of the screen.
            //if it does, put it on the next line.
            if(currWordLen + pos + spaceLen > pixelWidth)
            {
                pos = 0;
                if(lastSpaceIndex != 0)
                {
                    if((currWordLen != 0 && lastChar))
                        s.insert(lastSpaceIndex, "\n");
                    else
                        s[lastSpaceIndex] = '\n';
                }
            }
            pos += currWordLen + spaceLen;
            lastSpaceIndex = i;
            currWordLen = 0;
        }
        //otherwise, if there is a dash character, replace it with a newline. If there isn't, add the character to the current word and continue
        else
        {
            if(s[i] == '-')
            {
                s[i] = '\n';
                pos = 0;
            }
            else
            {
                unsigned int len = characters[s[i]].advance >> 6;    
                currWordLen += len;
            }
        }
    }
    return s;
}

//renders the buttons and text that belong to the instructions menu
void drawInstructionMenu()
{
    saveMenuBox.color = vec3(0.0f,0.0f,0.0f);
    drawRectangle(saveMenuBox);
    saveMenuBox.color = vec3(0.8f,0.8f,0.8f);
    //draw the instructions text
    std::string nText = instructionsText;
    nText.erase(std::remove(nText.begin(), nText.end(), '\n'), nText.end());
    std::string formattedText = formatText(nText);
    textInfo fTextInfo = getTextInfo(nText, 1.0f);
    drawText(formattedText, vec3(1.0f,1.0f,1.0f), 0.0f, scrollAmount +  pixelHeight - (fTextInfo.maxTop - fTextInfo.minBottom), 1.0f);

    //draw the back button (goes back to main menu)
    buttonData& backBtn = buttons[10];
    textInfo backInfo = getTextInfo(backBtn.text, 1.0f);
    backBtn.scale = vec2(pixelWidth, backInfo.maxTop - backInfo.minBottom);
    backBtn.position = vec2(0,0);
    drawButton(backBtn);
    
}


//draw the buttons that belong to the save menu
void drawSaveMenu()
{
    //draw box behind everything
    //draw heading (text + new save button)
    std::string text = "SAVE WORLD";
    textInfo headerInfo = getTextInfo(text, 1.0f);
    float x = pixelWidth/2.0f - headerInfo.totalWidth/2.0f;
    float y = menuTop - headerInfo.maxTop;

    drawRectangle(saveMenuBox);
    
    //draw every save (name + save + delete)
    float offset = -75.0f;
    float belowHeader = menuTop - (headerInfo.maxTop - headerInfo.minBottom) - 50.0f;
    float xPos = 25.0f;
    float yPos = offset + belowHeader;

    //now we render the "new save" button that creates a new save file
    bool renderedCurrFile = false;
    for(int i = 0; i < nameButtons.size(); i++)
    {
        glm::vec2 dims;
        dims.x = pixelWidth;
        dims.y = pixelHeight;

        
        buttonData& nameButton = nameButtons[i];
        std::string currName = nameButton.text;
        yPos = offset + belowHeader;
        offset -= 100.0f;
        textInfo strInfo = getTextInfo(currName, 1.0f);
        float textMid = yPos + strInfo.minBottom + (strInfo.maxTop - strInfo.minBottom)/2.0f;
        nameButton.scale = vec2(strInfo.totalWidth, 80.0f);

        nameButton.position = vec2(menuLeft, textMid + scrollAmount);
        drawButton(nameButton);

        if(currName != currFile || renderedCurrFile)
            continue;
        std::cout << currFile << " : currfile\n";
        renderedCurrFile = true;
        glm::vec2 buttonSize = (screenCoordsToNDC(80.0f, 40.0f) + glm::vec2(1.0f,1.0f))/0.5f;
        float buttonWidth = 2*(buttonSize.x * 0.5f) * dims.x/2.0f;
        float buttonPosXPix = xPos + strInfo.totalWidth + buttonWidth/2.0f;

        buttonData& saveButton = buttons[4];
        buttonData& deleteSaveButton = buttons[3];
        saveButton.scale = vec2(160.0f, 80.0f);
        saveButton.position = vec2(xPos + strInfo.totalWidth, textMid + scrollAmount);
        drawButton(saveButton);

        //draw load button
        buttonData& loadButton = buttons[5];
        float loadButtonPosXPix = buttonPosXPix + buttonWidth/2.0f + 10.0f;
        loadButton.scale = saveButton.scale;
        loadButton.position = vec2(loadButtonPosXPix, textMid + scrollAmount);
        drawButton(loadButton);

        float deletePosXPix = loadButtonPosXPix + buttonWidth + 10.0f;
        deleteSaveButton.scale = vec2(80.0f, 80.0f);
        deleteSaveButton.position = vec2(deletePosXPix, textMid + scrollAmount);

        drawButton(deleteSaveButton);
    }
    buttonData& newSaveButton = buttons[6];
    newSaveButton.scale = vec2(320.0f, 80.0f);
    newSaveButton.position = vec2(xPos, yPos - 70.0f + scrollAmount);
    newSaveButton.position.x = pixelWidth/2.0f - newSaveButton.scale.x/2.0f;
    drawButton(newSaveButton);
    glViewport(0, 0, pixelWidth, pixelHeight);

    saveMenuHeaderBox.scale = buttonScale(pixelWidth, -belowHeader - 50.0f + menuTop);
    saveMenuHeaderBox.position = buttonPosition(saveMenuHeaderBox.scale, 0.0f, belowHeader + 50.0f);

    drawRectangle(saveMenuHeaderBox);
    drawText(text, glm::vec3(0.0f,0.8f,0.2f), x, y, 1.0f);
}
//render a single colored rectangle
void drawRectangle(rectangleData& rectangle)
{
    if(rectangleID == NULL)
    {
        rectangleProgram.createProgram();
        rectangleID = 1;
    }

    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonID);
    glUseProgram(rectangleProgram.programID);

    glm::vec3 vertData[6] = 
    {
        {},
        {},
        {},
        {},
        {},
        {}
    };
    // glBufferSubData(GL_ARRAY_BUFFER, 0, GL_FLOAT * 3 * 6, (void*)vertData);
    //now we will set the vertices
    //set uniforms
    glUniform2f(glGetUniformLocation(rectangleProgram.programID, "scale"), rectangle.scale.x, rectangle.scale.y);
    glUniform2f(glGetUniformLocation(rectangleProgram.programID, "position"), rectangle.position.x, rectangle.position.y);
    glUniform3f(glGetUniformLocation(rectangleProgram.programID, "color"), rectangle.color.r, rectangle.color.g, rectangle.color.b);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//get the pixel dimensions of the window
glm::vec2 getViewportDims()
{
    GLint buffer[4];
    glGetIntegerv(GL_VIEWPORT, buffer);
    return (glm::vec2(buffer[2], buffer[3]));
}

//render a button
void drawButton(buttonData& button)
{
    if(buttonID == NULL)
    {
        glGenVertexArrays(1, &buttonVAO);
        glBindVertexArray(buttonVAO);
        buttonProgram.createProgram();
        //create shader program
        glGenBuffers(1, &buttonID);
        glBindBuffer(GL_ARRAY_BUFFER, buttonID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 6, buttonPts, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*5,(void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*5, (void*)sizeof(vec3));

        //we also create the rectangle shader program so we can have rectangle without texture
        rectangleProgram.createProgram();
    }
    glActiveTexture(button.tex.textureUnit);
    glBindTexture(GL_TEXTURE_2D, button.tex.textureID);
    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonID);
    glUseProgram(buttonProgram.programID);
    
    vec2 bScale = buttonScale(button.scale.x, button.scale.y);
    vec2 bPos = buttonPosition(bScale, button.position.x, button.position.y);
    //set uniforms (globals in glsl)
    glUniform2f(glGetUniformLocation(buttonProgram.programID, "scale"), bScale.x, bScale.y);
    glUniform2f(glGetUniformLocation(buttonProgram.programID, "position"), bPos.x, bPos.y);
    glUniform3f(glGetUniformLocation(buttonProgram.programID, "color"), button.color.r, button.color.g, button.color.b);
    glm::vec3 texMultiplier;
    //if the button has a texture, don't render it with it's color member variable, only the texture.
    if(button.tex.textureUnit == -1)
    {
        texMultiplier = glm::vec3(0.0f,0.0f,0.0f);
    }
    else
    {
        texMultiplier = glm::vec3(1.0f,1.0f,1.0f);
        glUniform1i(glGetUniformLocation(buttonProgram.programID, "myTexture"), (unsigned int)button.tex.textureUnit - GL_TEXTURE0);
    }
    vec3 tm = texMultiplier;
    glUniform3f(glGetUniformLocation(buttonProgram.programID, "texMultiplier"),  tm.x, tm.y, tm.z);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //if the button has text in it, we must render this in the center of the button
    if(button.useText == true)
    {
        textInfo info = getTextInfo(button.text, 1.0f);
        glm::vec2 dims = getViewportDims();
        glm::vec2 buttonPosPixels = vec2(button.position.x, button.position.y);
        float textMidOffsetX = info.totalWidth/2.0f;
        float textX = button.scale.x/2.0f - textMidOffsetX;
        float textMidOffsetY = (info.maxTop - info.minBottom)/2.0f;
        float textY = button.scale.y/2.0f - textMidOffsetY;
        drawText(button.text, button.textColor, buttonPosPixels.x + textX, buttonPosPixels.y + textY, 1.0f);
    }
}

//render an empty square frame on the screen
void drawSquareFrame(vec3* pts)
{
    if(squareFrameID == NULL)
    {
        glGenVertexArrays(1, &squareFrameVAO);
        glBindVertexArray(squareFrameVAO);
        squareFrameProgram.createProgram();
        //create shader program
        glGenBuffers(1, &squareFrameID);
        glBindBuffer(GL_ARRAY_BUFFER, squareFrameID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, pts, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*3,(void*)0);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, squareFrameID);
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, sizeof(float)*3*4, pts);
    }
    glBindVertexArray(squareFrameVAO);
    glUseProgram(squareFrameProgram.programID);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

//render a point on the screen
void drawPoint(glm::vec3 screenPos, glm::vec3 color)
{
    float bufferData[] = {screenPos[0], screenPos[1], screenPos[2], color[0], color[1], color[2]};
    if(pointID == NULL)
    {
        std::cout << "init point data\n";

        //set up point VAO and VBO. send vertex and color to gpu
        glGenVertexArrays(1, &pointVAO);
        glBindVertexArray(pointVAO);
        pointProgram.createProgram();
        glGenBuffers(1, &pointID);
        std::cout << "ID" << pointID << '\n';
        glBindBuffer(GL_ARRAY_BUFFER, pointID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, &bufferData, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 3));
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, pointID);
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, sizeof(bufferData), &(bufferData[0]));
    }
    
    glBindVertexArray(pointVAO);
    glUseProgram(pointProgram.programID);
    //set uniforms
    glUniformMatrix4fv(glGetUniformLocation(pointProgram.programID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(pointProgram.programID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glDrawArrays(GL_POINTS, 0, 1);
}