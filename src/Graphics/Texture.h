#pragma once
#include "pch.h"
#include "GPUBuffer.h"


namespace bdr
{
    class Renderer;

    void reset(Texture& texture);

    TextureHandle createTextureFromFile(
        Renderer& renderer,
        const std::string& filePath,
        const TextureCreationInfo& createInfo
    );
}