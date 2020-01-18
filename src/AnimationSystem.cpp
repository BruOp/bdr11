#include "pch.h"
#include "AnimationSystem.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    struct InterpolationInfo
    {
        uint32_t previousIdx = 0;
        uint32_t nextIdx = 1;
        // Interpolation Coefficient
        float t = 0.0f;
    };

    // Increments the channel's currentInputIdx, if necessary
    template<typename ChannelT>
    void updateChannel(ChannelT& channel, const float animationTime)
    {
        float deltaT = (channel.maxInput - channel.input[0]) / float(channel.input.size() - 1);
        channel.currentInputIdx = static_cast<uint32_t>(floorf(animationTime / deltaT));
    };

    template<typename ChannelT>
    InterpolationInfo calcInterpolationInfo(ChannelT& channel, const float animationTime)
    {
        uint32_t nextIdx = channel.currentInputIdx + 1;
        float previousTime = channel.input[channel.currentInputIdx];
        float nextTime = channel.input[nextIdx];
        // Interpolation Value
        float t = (animationTime - previousTime) / (nextTime - previousTime);
        return {
            channel.currentInputIdx,
            nextIdx,
            t
        };
    }

    void updateAnimation(ECSRegistry& registry, Animation& animation, const float currentTime)
    {
        if (animation.playingState == Animation::State::Off) {
            return;
        }
        else if (animation.playingState == Animation::State::Resetting) {
            animation.playingState = Animation::State::Off;
            animation.startTime = currentTime;
        }

        const float timeSinceStart = currentTime - animation.startTime;
        for (auto& channel : animation.rotationChannels) {
            const float animationTime = fmod(timeSinceStart, channel.maxInput);

            updateChannel(channel, animationTime);
            InterpolationInfo info = calcInterpolationInfo(channel, animationTime);

            Transform& transform = registry.transforms[channel.targetEntity];
            const Quaternion& previous = channel.output[info.previousIdx];
            const Quaternion& next = channel.output[info.nextIdx];
            transform.rotation = Quaternion::Slerp(previous, next, info.t);
        }

        for (auto& channel : animation.translationChannels) {
            const float animationTime = fmod(timeSinceStart, channel.maxInput);

            updateChannel(channel, animationTime);
            InterpolationInfo info = calcInterpolationInfo(channel, animationTime);

            Transform& transform = registry.transforms[channel.targetEntity];
            const Vector3& previous = channel.output[info.previousIdx];
            const Vector3& next = channel.output[info.nextIdx];
            transform.translation = Vector3::Lerp(previous, next, info.t);
        }

        for (auto& channel : animation.scaleChannels) {
            const float animationTime = fmod(timeSinceStart, channel.maxInput);

            updateChannel(channel, animationTime);
            InterpolationInfo info = calcInterpolationInfo(channel, animationTime);

            Transform& transform = registry.transforms[channel.targetEntity];
            const Vector3& previous = channel.output[info.previousIdx];
            const Vector3& next = channel.output[info.nextIdx];
            transform.scale = Vector3::Lerp(previous, next, info.t);
        }
    }

    void updateMatrices(ECSRegistry& registry)
    {
        for (size_t entity = 0; entity < registry.numEntities; entity++) {
            const uint32_t cmpMask = registry.cmpMasks[entity];
            Matrix& local = registry.localMatrices[entity];
            if (cmpMask & CmpMasks::TRANSFORM) {
                const Transform& transform = registry.transforms[entity];
                local = getMatrixFromTransform(transform);
                registry.localMatrices[entity] = local;
            }

            if (cmpMask & CmpMasks::PARENT) {
                const uint32_t parent = registry.parents[entity];
                registry.globalMatrices[entity] = local * registry.globalMatrices[parent];
            }
            else {
                registry.globalMatrices[entity] = local;
            }
        }
    }

    void copyDrawData(ECSRegistry& registry)
    {
        for (size_t entity = 0; entity < registry.numEntities; entity++) {
            const uint32_t cmpMask = registry.cmpMasks[entity];
            if (cmpMask & CmpMasks::MESH) {
                registry.drawConstants[entity].model = registry.globalMatrices[entity].Transpose();
                registry.drawConstants[entity].invModel = registry.globalMatrices[entity].Invert();
                registry.drawConstants[entity].invModel._14 = 0;
                registry.drawConstants[entity].invModel._24 = 0;
                registry.drawConstants[entity].invModel._34 = 0;
            }
        }
    }
}