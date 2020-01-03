#pragma once
#include "Animation.h"
#include "ECSRegistry.h"

namespace bdr
{
    void updateAnimation(ECSRegistry& registry, Animation& animation, const float currentTime);

    void updateMatrices(ECSRegistry& registry);

    void copyDrawData(ECSRegistry& registry);
}