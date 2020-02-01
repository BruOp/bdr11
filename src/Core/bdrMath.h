#pragma once
#include "pch.h"


namespace bdr
{
    enum TransformType : uint8_t
    {
        Rotation = 1,
        Translation = 2,
        Scale = 4,
        // Weights not supported
    };

    struct Transform
    {
        DirectX::SimpleMath::Quaternion rotation;
        DirectX::SimpleMath::Vector3 translation;
        uint32_t mask;
        DirectX::SimpleMath::Vector3 scale;
    };

    DirectX::SimpleMath::Matrix getMatrixFromTransform(const Transform& transform);
}