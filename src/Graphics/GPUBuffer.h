#pragma once
#include "pch.h"
#include "Resources.h"

namespace bdr
{
    void reset(GPUBuffer& gpuBuffer);

    DXGI_FORMAT mapFormatToDXGI(const BufferFormat bufferFormat);

    uint32_t getByteSize(const BufferCreationInfo& createInfo);

    uint32_t getByteSize(const BufferFormat& format);

    GPUBuffer createBuffer(ID3D11Device* pDevice, const void* data, const BufferCreationInfo& createInfo);

}
