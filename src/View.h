#pragma once
#include "pch.h"

namespace bdr
{
    class Renderer;
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

    class View
    {
    public:
        std::string name = "";
        Scene* scene = nullptr;
        ViewType type = ViewType::Unknown;

        inline Camera const* getCamera() const
        {
            if (type == ViewType::Camera) {
                return perspectiveProvider.camera;
            }
            else {
                return nullptr;
            }
        };

        void setCamera(const Camera* camera);

    private:
        union PerspectiveProvider
        {
            Camera const* camera;
        } perspectiveProvider = { nullptr };
    };

    void setConstants(Renderer* renderer, const View& view);
}