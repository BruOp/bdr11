#pragma once
#include "pch.h"

namespace bdr
{
    class Scene;
    struct Camera;

    struct ViewConstants
    {
        DirectX::SimpleMath::Matrix viewTransform;
        DirectX::SimpleMath::Matrix perspectiveTransform;
        DirectX::SimpleMath::Matrix VP;
    };

    struct View
    {
        enum Type : uint32_t
        {
            Unknown = 0,
            CameraType,
            LightType
        };

        std::string name = "";
        Scene* scene = nullptr;
        union PerspectiveProvider
        {
            Camera const* camera;
        } perspectiveProvider;
        Type type = Unknown;
    };
}