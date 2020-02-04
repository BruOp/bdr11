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

    glm::mat4 math::perspective(const float fov, const float aspectRatio, const float _near, const float _far)
    {
        constexpr glm::mat4 glToDxCoords{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.5f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return glToDxCoords * glm::perspectiveFov(fov, aspectRatio, 1.0f, _near, _far);
    }
}

