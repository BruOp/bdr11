#pragma once
#include "Animation.h"
#include "ECSRegistry.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    void updateAnimation(ECSRegistry& registry, const Animation& animation, const float currentTime)
    {
        for (const auto& channel : animation.channels) {
            const float animationTime = fmod(currentTime, channel.maxInput);
            size_t nextIdx = 0;
            while (nextIdx < channel.input.size() && channel.input[nextIdx] < animationTime) {
                ++nextIdx;
            }
            size_t previousIdx = nextIdx == 0 ? 0 : nextIdx - 1;

            float previousTime = channel.input[previousIdx];
            float nextTime = channel.input[nextIdx];
            // Interpolation Value
            float t = (animationTime - previousTime) / (nextTime - previousTime);

            Transform& transform = registry.transforms[channel.targetEntity];
            switch (channel.targetType) {
            case TransformType::Scale:
            {
                const Vector3 previous = Vector3{ channel.output[previousIdx] };
                const Vector3 next = Vector3{ channel.output[nextIdx] };
                transform.scale = Vector3::Lerp(previous, next, t);
                break;
            }
            case TransformType::Rotation:
            {
                const Quaternion& previousRot = channel.output[previousIdx];
                const Quaternion& nextRot = channel.output[nextIdx];
                transform.rotation = Quaternion::Slerp(previousRot, nextRot, t);
                break;
            }
            case TransformType::Translation:
            {
                const Vector3 previous = Vector3{ channel.output[previousIdx] };
                const Vector3 next = Vector3{ channel.output[nextIdx] };
                transform.translation = Vector3::Lerp(previous, next, t);
                break;
            }
            //default:
            //    Utility::Printf("Skipping weights animation node");
            //    continue;
            }
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
}