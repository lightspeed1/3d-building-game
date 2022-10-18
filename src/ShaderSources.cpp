#include "include/ShaderSources.h"
const char* cubeVertShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in mat4 rotationMatrix;
layout (location = 6) in vec3 translate;
layout (location = 7) in vec3 color_;
layout (location = 8) in vec3 scale;
layout (location = 9) in mat4 normalMatrix;
layout (location = 12) in int texUnit;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cubeNormals[6];
// out vec3 texCoords_;
out vec3 translate2;
out int instID;
out vec3 color;

void main()
{
    color = color_;
    translate2 = translate;
    mat4 model = rotationMatrix;
    model[3] = vec4(translate, 1.0f);
    gl_Position = projection * view * model * vec4(scale * vertexPos, 1.0f);
    instID = gl_InstanceID;
    // texCoords_ = texCoords;
}
)";
const char* cubeFragShaderSource = R"(
#version 460 core
// in vec3 texCoords_;
// uniform sampler2D currTexture;
in vec3 translate2;
in vec3 color;
in flat int instID;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
out vec4 FragColor;

void main()
{
    // FragColor = mix(texture(currTexture, texCoords_), 0.5f);
    // FragColor = vec4(instID, 0.0f, 0.0f, 1.0f);
    FragColor = vec4(color, 1.0f);
}
)";