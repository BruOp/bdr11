#pragma once
#include "pch.h"

namespace bdr
{
    class Scene;
    struct Camera;

    enum class ViewType : uint32_t
    {
        Unknown = 0,
        Camera,
        Light
    };

    struct ViewConstants
    {
        DirectX::SimpleMath::Matrix viewTransform;
        DirectX::SimpleMath::Matrix perspectiveTransform;
        DirectX::SimpleMath::Matrix VP;
    };

    struct View
    {
        std::string name = "";
        Scene* scene = nullptr;
        union PerspectiveProvider
        {
            Camera const* camera;
        } perspectiveProvider = { nullptr };
        ViewType type = ViewType::Unknown;
    };
}