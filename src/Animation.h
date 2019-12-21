#pragma once
#include "pch.h"


namespace bdr
{
    class ECSRegistry;

    enum TransformType : uint8_t
    {
        Rotation = 1,
        Translation = 2,
        Scale = 4,
        // Weights not supported
    };

    struct Animation
    {
        enum State : uint8_t
        {
            Off = 0u,
            Resetting,
            Playing,
        };
        enum InterpolationType : uint8_t
        {
            Linear = 1,
            Step = 2,
            CubicSpline = 4,
        };

        template<typename T>
        struct Channel
        {
            uint32_t targetEntity = UINT32_MAX;
            uint32_t currentInputIdx = 0u;
            float maxInput = 0.0f;
            InterpolationType interpolationType = InterpolationType::Linear;
            std::vector<float> input;
            std::vector<T> output;
        };

        float startTime = 0.0f;
        State playingState = State::Off;
        typedef Channel<DirectX::SimpleMath::Vector3> TranslationChannel;
        typedef Channel<DirectX::SimpleMath::Vector4> RotationChannel;
        typedef Channel<DirectX::SimpleMath::Vector3> ScaleChannel;

        std::vector<TranslationChannel> translationChannels;
        std::vector<RotationChannel> rotationChannels;
        std::vector<ScaleChannel> scaleChannels;
    };

    inline void addTranslationChannel(Animation& animation, Animation::TranslationChannel&& channel)
    {
        animation.translationChannels.push_back(std::forward<Animation::TranslationChannel>(channel));
    }

    inline void addRotationChannel(Animation& animation, Animation::RotationChannel&& channel)
    {
        animation.rotationChannels.push_back(std::forward<Animation::RotationChannel>(channel));
    }

    inline void addScaleChannel(Animation& animation, Animation::ScaleChannel&& channel)
    {
        animation.scaleChannels.push_back(std::forward<Animation::ScaleChannel>(channel));
    }

    struct Skin
    {
        std::vector<uint32_t> jointEntities;
        std::vector<DirectX::SimpleMath::Matrix> inverseBindMatrices;
    };
}