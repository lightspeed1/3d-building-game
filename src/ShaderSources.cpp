#include "include/ShaderSources.h"

//globals for shader source files to be compiled and sent to gpu in order to render parts
const char* cubeVertShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in mat4 rotationMatrix;
layout (location = 6) in vec3 translate;
layout (location = 7) in vec3 color_;
layout (location = 8) in vec3 scale;
layout (location = 9) in mat3 normalMatrix;
layout (location = 12) in int texUnit;
layout (location = 13) in float normInd;
uniform vec3 norms[21];
uniform int offset;
uniform mat4 view;
uniform mat4 projection;
// uniform vec3 cubeNormals[6];
// out vec3 texCoords_;
out vec3 translate2;
// out int instID;
out vec3 color;

out vec3 worldPos;
out flat vec3 norm;
void main()
{
    color = color_;
    translate2 = translate;
    mat4 model = rotationMatrix;
    model[3] = vec4(translate, 1.0f);
    gl_Position = projection * view * model * vec4(scale * vertexPos, 1.0f);
    worldPos =  vec3(model * vec4(vertexPos, 1.0f));
    // instID = gl_InstanceID;
    // texCoords_ = texCoords;
    vec3 orig = norms[offset + int(normInd)];
    norm = normalize(normalMatrix * orig);
}
)";
const char* cubeFragShaderSource = R"(
#version 460 core
// in vec3 texCoords_;
// uniform sampler2D currTexture;
in vec3 translate2;
in vec3 color;
// in flat int instID;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
out vec4 FragColor;

void main()
{
    // FragColor = mix(texture(currTexture, texCoords_), 0.5f);
    // FragColor = vec4(instID, 0.0f, 0.0f, 1.0f);
    FragColor = vec4(color, 1.0f);
}
)";
const char* lightingFragShader = R"(
#version 460 core
// in vec3 translate2;
// in flat int instID;
// layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
// in vec2 myTexCoords;
in vec3 color;
in flat vec3 norm;
in vec3 worldPos;

// uniform float mixAmount;
uniform vec3 lightPos;
uniform vec3 camPos;

out vec4 FragColor;
void main(){
    //diffuse lighting
    vec3 lPos = vec3(0.0f, 0.0f, 3.0f);
    vec3 lightColor = vec3(0.8f, 0.8f, 0.8f);
    vec3 vertexColor = color;
    vec3 lightDirection = normalize(lightPos - worldPos);
    float ambientIntensity = 0.7f;
    vec3 ambientColor = ambientIntensity * lightColor;
    vec3 diffuseLight = (lightColor * max(dot(norm, lightDirection), 0.0f));
    //specular lighting
    vec3 camTarget = normalize(camPos - worldPos);
    float specularIntensity = 0.4f;
    vec3 lightReflection = reflect(-lightDirection, norm);
    float specularValue = pow(max(dot(camTarget, lightReflection), 0.0),64);
    vec3 specularLight = specularValue * specularIntensity * lightColor;

    FragColor = vec4((ambientColor + diffuseLight + specularLight) * vertexColor, 1.0f);

    // FragColor = vec4(lightColor * max(dot(norm, normalize(lPos - worldPos)), 0.0f), 1.0f);

    // FragColor = mix(texture(myTexture, myTexCoords), texture(myOtherTexture, myTexCoords), mixAmount);
}
)";

const char* outlineFragShaderSource = R"(
#version 460 core
// in vec3 texCoords_;
// uniform sampler2D currTexture;
in vec3 translate2;
in vec3 color;
// in flat int instID;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f,0.0f, 1.0f, 1.0f);
}
)";