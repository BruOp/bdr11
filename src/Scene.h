#pragma once
#include "pch.h"

#include <vector>
#include <array>

#include "DeviceResources.h"
#include "ECSRegistry.h"
#include "Animation.h"
#include "Camera.h"

namespace bdr
{
    class Scene
    {
    public:
        Scene() = default;
        ~Scene()
        {
            reset();
        }


        UNCOPIABLE(Scene);
        UNMOVABLE(Scene);

        inline void reset()
        {
            registry.clearComponentData();
            skins = std::vector<Skin>();
            animations = std::vector<Animation>();
            cameras = std::vector<Camera>();
        }

        inline operator ECSRegistry& ()
        {
            return registry;
        }

        ECSRegistry registry;
        std::vector<Skin> skins;
        std::vector<Animation> animations;
        std::vector<Camera> cameras;
    };

    Camera& getCamera(Scene& scene, const uint32_t cameraId);

    uint32_t createPerspectiveCamera(Scene& scene, float fov, float aspectRatio, float _near, float _far);
}
