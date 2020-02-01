#pragma once
#include "pch.h"
#include "GPUBuffer.h"


namespace bdr
{

    struct TextureCreationInfo
    {
        uint32_t dims[2] = { 0, 0 };
        uint8_t usage = 0;
    };

    struct Texture
    {
        uint32_t numLayers = 0;
        uint32_t numMips = 0;
        uint32_t dims[2] = { 0, 0 };
        uint8_t usage = 0;
        BufferType srvType = BufferType::Default;
        BufferType uavType = BufferType::Default;
        BufferFormat format = BufferFormat::INVALID;
        ID3D11Resource* texture = nullptr;
        ID3D11UnorderedAccessView* uav = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        ID3D11SamplerState* sampler = nullptr;
        void reset();

        static Texture createFromFile(ID3D11Device* pDevice, const std::string& fileName, const TextureCreationInfo& createInfo);
    };

    //Texture createTexture2D(ID3D11Device* pDevice, const void* data, const TextureCreationInfo& createInfo);
}