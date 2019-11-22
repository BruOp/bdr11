#pragma once
#include "pch.h"
#include "NodeList.h"

namespace bdr
{

    
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
            int32_t targetNodeIdx;
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
        std::vector<int32_t> jointIndices;
        std::vector<DirectX::SimpleMath::Matrix> inverseBindMatrices;
    };

    void updateAnimation(NodeList& nodeList, const Animation& animation, const float currentTime);

}