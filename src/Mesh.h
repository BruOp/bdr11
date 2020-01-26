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
        static constexpr size_t maxAttrCount = 6u;
        GPUBuffer indexBuffer;
        // Different handle for each "stream" of vertex attributes
        // 0 - Position
        // 1 - Normal
        // 2 - TexCoord0
        // 3 - Weights
        // 4 - BlendIndices
        // 5 - Color
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

    struct MeshCreationInfo
    {
        uint8_t const* indexData = nullptr;
        BufferFormat indexFormat = BufferFormat::INVALID;
        // 0 - Position
        // 1 - Normal
        // 2 - TexCoord0
        // 3 - Weights
        // 4 - BlendIndices
        // 5 - Color
        uint8_t* data[Mesh::maxAttrCount] = { nullptr };
        BufferFormat bufferFormats[Mesh::maxAttrCount] = { BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID };
        uint8_t presentAttributesMask = 0;
        size_t strides[Mesh::maxAttrCount] = { 0 };
        size_t numIndices = 0;
        size_t numVertices = 0;
    };

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[]);
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[]);
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[]);

    void addPositionAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format);
    void addColorAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format);
}