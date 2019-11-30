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

    struct Animation
    {
        enum InterpolationType : uint8_t
        {
            Linear = 1,
            Step = 2,
            CubicSpline = 4,
        };

        struct Channel
        {
            uint32_t targetEntity;
            float maxInput;
            TransformType targetType;
            InterpolationType interpolationType;
            std::vector<float> input;
            std::vector<DirectX::SimpleMath::Vector4> output;
        };

        std::vector<Channel> channels;
    };

    struct Skin
    {
        std::vector<uint32_t> jointEntities;
        std::vector<DirectX::SimpleMath::Matrix> inverseBindMatrices;
    };

    //void updateAnimation(NodeList& nodeList, const Animation& animation, const float currentTime);

}