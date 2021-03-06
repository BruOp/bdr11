#pragma once
#include "pch.h"

#include "Core/bdrMath.h"

namespace bdr
{
    class ECSRegistry;

    struct Animation
    {
        enum class State : uint8_t
        {
            Off = 0u,
            Resetting,
            Playing,
        };
        enum class InterpolationType : uint8_t
        {
            Linear = 0,
            Step,
            CubicSpline,
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
        typedef Channel<glm::vec3> TranslationChannel;
        typedef Channel<glm::quat> RotationChannel;
        typedef Channel<glm::vec3> ScaleChannel;

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
        std::vector<glm::mat4> inverseBindMatrices;
    };
}