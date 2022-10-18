#pragma once
#define GLEW_STATIC 1
#include "glew.h"
#include "../../glm/glm/glm.hpp"
#include "../../glm/glm/gtc/matrix_transform.hpp"
#include "../../glm/glm/gtc/type_ptr.hpp"

class PartData
{
    public:
        glm::mat4 rotationMatrix{1.0f};
        glm::vec3 translate{0.0f, 0.0f, 1.0f};
        glm::vec3 color{0.0f, 0.0f, 1.0f};
        glm::vec3 scaleVector{1.0f, 1.0f, 1.0f};
        glm::mat3 normalMatrix{1.0f};
        int texUnit;
        void scale(glm::vec3 vec);
        void rotate(glm::vec3 axis, float degrees);
};