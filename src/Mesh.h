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
        // No mesh will contain all 6 possible mesh attributes
        static constexpr size_t maxAttrCount = 5u;

        ID3D11Buffer* vertexBuffers[maxAttrCount] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        ID3D11UnorderedAccessView* uavs[maxAttrCount] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        ID3D11ShaderResourceView* srvs[maxAttrCount] = { nullptr, nullptr, nullptr, nullptr, nullptr };
        ID3D11Buffer* indexBuffer = nullptr;
        DXGI_FORMAT indexFormat;
        uint32_t inputLayoutHandle = UINT32_MAX;
        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
        uint32_t strides[maxAttrCount];
        uint32_t preskinMeshIdx = UINT32_MAX;
        uint32_t subMeshIdx = UINT32_MAX;
        uint8_t presentAttributesMask;
        uint8_t numPresentAttr;

        void destroy()
        {
            for (auto srv : srvs) {
                srv != nullptr && srv->Release();
            }
            for (auto uav : uavs) {
                uav != nullptr && uav->Release();
            }
            for (auto buffer : vertexBuffers) {
                buffer != nullptr && buffer->Release();
            }
            if (indexBuffer != nullptr) {
                indexBuffer->Release();
            }
        }
    };
}