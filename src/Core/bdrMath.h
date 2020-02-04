#pragma once
#include "pch.h"

namespace bdr
{
    namespace math
    {
        constexpr float PI = glm::pi<float>();
        constexpr float HALF_PI = glm::half_pi<float>();
        constexpr float TWO_PI = glm::two_pi<float>();

        constexpr glm::vec3 up{ 0.0f, 1.0f, 0.0f };

        inline glm::vec3 getRight(const glm::mat4& transform)
        {
            return glm::column(transform, 1);
        }

        inline glm::vec3 getTranslation(const glm::mat4& transform)
        {
            return glm::column(transform, 3);
        }

        glm::mat4 perspective(const float fov, const float width, const float height, const float _near, const float _far);
    }

    enum TransformType : uint8_t
    {
        Rotation = 1,
        Translation = 2,
        Scale = 4,
        // Weights not supported
    };

    struct Transform
    {
        glm::quat rotation;
        glm::vec3 translation;
        uint32_t mask;
        glm::vec3 scale;
    };

    glm::mat4 getMatrixFromTransform(const Transform& transform);
}