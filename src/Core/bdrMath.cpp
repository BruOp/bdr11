#include "pch.h"
#include "bdrMath.h"


namespace bdr
{

    glm::mat4 getMatrixFromTransform(const Transform& transform)
    {
        glm::mat4 localTransform = glm::identity<glm::mat4>();
        if (transform.mask & TransformType::Scale) {
            localTransform = glm::scale(localTransform, transform.scale);
        }
        if (transform.mask & TransformType::Rotation) {
            localTransform *= glm::mat4_cast(transform.rotation);
        }
        if (transform.mask & TransformType::Translation) {
            localTransform = glm::translate(localTransform, transform.translation);
        }
        return localTransform;
    }

    glm::mat4 math::perspective(const float fov, const float width, const float height, const float zNear, const float zFar)
    {
        constexpr glm::mat4 glToDxCoords{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.5f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        const float h = glm::cos(0.5f * fov) / glm::sin(0.5f * fov);
        const float w = h * height / width;

        glm::mat4 result(0.0f);
        result[0][0] = w;
        result[1][1] = h;
        result[2][2] = -(zFar) / (zNear - zFar) - 1;
        result[2][3] = -1.0f;
        result[3][2] = -(zNear * zFar) / (zNear - zFar);
        return result;
    }
}

