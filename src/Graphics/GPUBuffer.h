#pragma once
#include "pch.h"
#include "Resources.h"

namespace bdr
{
    class Renderer;

    void reset(GPUBuffer& gpuBuffer);

    DXGI_FORMAT mapFormatToDXGI(const BufferFormat bufferFormat);

    uint32_t getByteSize(const BufferCreationInfo& createInfo);

    uint32_t getByteSize(const BufferFormat& format);

    GPUBuffer createBuffer(ID3D11Device* pDevice, const void* data, const BufferCreationInfo& createInfo);

    GPUBuffer createStructuredBuffer(ID3D11Device* device, const uint32_t elementSize, const uint32_t numElements);
}
