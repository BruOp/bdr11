#pragma once
#include "pch.h"

#include "DXHelpers.h"
#include "Resources.h"

namespace bdr
{
    class Renderer;

    void reset(Material& material);

    struct PBRConstants
    {
        float baseColorFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float emissiveFactor[3] = { 0.0f, 0.0f, 0.0f };
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float alphaCutoff = 1.0f;
        uint32_t alphaMode = 0;
        float padding[53];
    };
    //static_assert(sizeof(PBRConstants) == sizeof(GenericMaterialData), "PBR Constants must be the same size as GenericMaterialData");

}

