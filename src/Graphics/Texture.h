#pragma once
#include "pch.h"
#include "GPUBuffer.h"


namespace bdr
{
    void reset(Texture& texture);

    Texture createFromFile(ID3D11Device* pDevice, const std::string& fileName, const TextureCreationInfo& createInfo);
}