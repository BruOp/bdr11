#pragma once
#include "pch.h"

namespace bdr
{
    enum MeshAttributes : uint8_t
    {
        POSITION = (1 << 0),
        NORMAL = (1 << 1),
        TEXCOORD = (1 << 2),
        TANGENT = (1 << 3),
        BLENDINDICES = (1 << 4),
        BLENDWEIGHT = (1 << 5),
    };

    struct Mesh
    {
        static constexpr size_t maxAttrCount = 6u;

        ID3D11Buffer* vertexBuffers[maxAttrCount] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        ID3D11Buffer* indexBuffer = nullptr;
        DXGI_FORMAT indexFormat;
        uint32_t inputLayoutHandle = UINT32_MAX;
        uint32_t indexCount = 0;
        uint32_t strides[maxAttrCount];
        uint8_t presentAttributesMask;
        uint8_t numPresentAttr;

        void destroy()
        {
            for (auto buffer : vertexBuffers) {
                buffer != nullptr && buffer->Release();
            }
            indexBuffer->Release();
        }
    };
}