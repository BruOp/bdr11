#pragma once
#include "pch.h"

#include <vector>
#include <array>

#include "DeviceResources.h"
#include "ECSRegistry.h"
#include "Animation.h"


namespace tinygltf
{
    struct Accessor;
    struct Primitive;
    struct Skin;
    struct Animation;
    class Model;
}

namespace bdr
{
    class Scene
    {
    public:
        ECSRegistry registry;
        std::vector<Skin> skins;
        std::vector<Animation> animations;

        void reset()
        {
            registry.clearComponentData();
        }
    };
}
