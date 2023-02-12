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
#include <algorithm>
#include "include/text.h"
#include "ft2build.h"
#include FT_FREETYPE_H
glm::mat4 orthoProj = glm::ortho(0.0f, pixelWidth, 0.0f, pixelHeight);

//shader sources for rectangle that characters will be drawn on
const char* charVertShader = R"(
#version 460 core
layout (location = 0) in vec4 vertex;
out vec2 texCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.x, vertex.y, 0.0, 1.0);
    texCoords = vec2(vertex.z, vertex.w);
}  
)";

const char* charFragShader = R"(
#version 460 core
in vec2 texCoords;
out vec4 FragColor;
uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 texPix = vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
    FragColor = vec4(textColor, 1.0) * texPix;
}
)";

ShaderProgram charProgram{charVertShader, charFragShader};
std::unordered_map<char, Character> characters;

unsigned int charVBO;
unsigned int charVAO;

int charTexUnit;


//creates textures for many characters, which will be able to be rendered to the screen
int setupFont()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR: Could not initialize free type library.\n";
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/comic.ttf", 0, &face))
    {
        std::cout << "ERROR: Cannot load font for free type.\n";
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    //no byte alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (char c = 0; c < 127; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR: Failed to load char info for free type.\n";
            continue;
        }
        //create texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        //only one color channel here
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x};
        characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //now we allocate space on the gpu for char vertices
    glGenVertexArrays(1, &charVAO);
    glGenBuffers(1, &charVBO);
    glBindVertexArray(charVAO);
    glBindBuffer(GL_ARRAY_BUFFER, charVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    charProgram.createProgram(); 
    charTexUnit = findOpenTexUnit();  
    return 0;
}

//get dimensions of string of characters
textInfo getTextInfo(std::string text, float scale)
{
    std::string::const_iterator c;
    float totalWidth = 0.0f, maxTop = FLT_MIN, minBottom = FLT_MAX;
    float x = 0.0f;
    float y = 0.0f;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        maxTop = std::max(maxTop, ypos + h);
        minBottom = std::min(minBottom, ypos);
        x += (ch.advance >> 6) * scale;
    }
    return(textInfo{x, maxTop, minBottom});
}

//renders text to the screen.
void drawText(std::string text, glm::vec3 color, float x, float y, float scale)
{
    float origX = x;
    //set uniforms for char rectangle shader program
    glUseProgram(charProgram.programID);
    glUniform1i(glGetUniformLocation(charProgram.programID, "text"), (unsigned int)charTexUnit - GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(charProgram.programID, "textColor"), color.x, color.y, color.z);
    auto ortho = glm::value_ptr(orthoProj);

    glUniformMatrix4fv(glGetUniformLocation(charProgram.programID, "projection"), 1, GL_FALSE, ortho);

    glActiveTexture(charTexUnit);
    glBindVertexArray(charVAO);
    //loop through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];
        //if current character is newline, move y position of subsequent characters down  
        if(*c == '\n' && *c != text[0])
        {
            y -= 50.0f;
            x = origX;
            continue;
        }
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        //set vertices for the rectangle on which the character texture will be rendered
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},            
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}           
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        //write vertices to gpu
        glBindBuffer(GL_ARRAY_BUFFER, charVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //render square with character texture
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //change x variable for next character (multiply advance of current character by 64 and add it to x)
        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}