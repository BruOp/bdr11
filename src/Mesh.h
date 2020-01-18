#pragma once
#include "pch.h"

#include "GPUBuffer.h"

namespace bdr
{
    enum MeshAttributes : uint8_t
    {
        POSITION = (1 << 0),
        NORMAL = (1 << 1),
        TEXCOORD = (1 << 2),
        BLENDINDICES = (1 << 3),
        BLENDWEIGHT = (1 << 4),
    };

    struct Mesh
    {
        // No mesh will contain all 6 possible mesh attributes
        static constexpr size_t maxAttrCount = 5u;
        GPUBuffer indexBuffer;
        GPUBuffer vertexBuffers[maxAttrCount];
        uint32_t inputLayoutHandle = UINT32_MAX;
        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
        uint32_t strides[maxAttrCount] = { 0 };
        uint32_t preskinMeshIdx = UINT32_MAX;
        uint8_t presentAttributesMask = 0;
        uint8_t numPresentAttr = 0;

        void reset()
        {
            for (auto& vertexBuffer : vertexBuffers) {
                vertexBuffer.reset();
            }
            indexBuffer.reset();
            numIndices = 0;
            numVertices = 0;
        }
    };

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[]);
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[]);
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[]);


}