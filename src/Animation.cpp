#include "pch.h"
#include "Animation.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    //void updateAnimation(NodeList& nodeList, const Animation& animation, const float currentTime)
    //{
    //    for (const auto& channel : animation.channels) {
    //        const float animationTime = fmod(currentTime, channel.maxInput);
    //        size_t nextIdx = 0;
    //        while (nextIdx < channel.input.size() && channel.input[nextIdx] < animationTime) {
    //            ++nextIdx;
    //        }
    //        size_t previousIdx = nextIdx == 0 ? channel.input.size() - 1 : nextIdx - 1;

    //        float previousTime = channel.input[previousIdx];
    //        float nextTime = channel.input[nextIdx];
    //        // Interpolation Value
    //        float t = (currentTime - previousTime) / (nextTime - previousTime);

    //        switch (channel.targetType) {
    //        case TransformType::Scale:
    //        {
    //            const Vector3 previous = Vector3{ channel.output[previousIdx] };
    //            const Vector3 next = Vector3{ channel.output[nextIdx] };
    //            nodeList.nodes[channel.targetNodeIdx].scale = Vector3::Lerp(previous, next, t);
    //            break;
    //        }
    //        case TransformType::Rotation:
    //        {
    //            const Quaternion& previousRot = channel.output[previousIdx];
    //            const Quaternion& nextRot = channel.output[nextIdx];
    //            nodeList.nodes[channel.targetNodeIdx].rotation = Quaternion::Slerp(previousRot, nextRot, t);
    //            break;
    //        }
    //        case TransformType::Translation:
    //        {
    //            const Vector3 previous = Vector3{ channel.output[previousIdx] };
    //            const Vector3 next = Vector3{ channel.output[nextIdx] };
    //            nodeList.nodes[channel.targetNodeIdx].translation = Vector3::Lerp(previous, next, t);
    //            break;
    //        }
    //        default:
    //            Utility::Printf("Skipping weights animation node");
    //            continue;
    //        }
    //    }

    //    updateNodes(nodeList);
    //}
}