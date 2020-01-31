#pragma once
#include "pch.h"

#include "GPUBuffer.h"

namespace bdr
{
    enum MeshAttribute : uint8_t
    {
        INVALID = 0,
        POSITION = (1 << 0),
        NORMAL = (1 << 1),
        TEXCOORD = (1 << 2),
        BLENDINDICES = (1 << 3),
        BLENDWEIGHT = (1 << 4),
        COLOR = (1 << 5),
    };

    struct Mesh
    {
        static constexpr size_t maxAttrCount = 6u;
        GPUBuffer indexBuffer;
        GPUBuffer vertexBuffers[maxAttrCount];
        ID3D11InputLayout* inputLayoutHandle = nullptr;
        uint32_t strides[maxAttrCount] = { 0 };
        MeshAttribute attributes[Mesh::maxAttrCount] = { MeshAttribute::INVALID };
        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
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
        uint8_t* data[Mesh::maxAttrCount] = { nullptr };
        BufferFormat bufferFormats[Mesh::maxAttrCount] = { BufferFormat::INVALID };
        MeshAttribute attributes[Mesh::maxAttrCount] = { MeshAttribute::INVALID };
        uint32_t strides[Mesh::maxAttrCount] = { 0 };
        uint8_t bufferUsages[Mesh::maxAttrCount] = { BufferUsage::Unused };
        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
        uint8_t numAttributes = 0;
        uint8_t presentAttributesMask = 0;
    };

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[]);
    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[]);
    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[]);

    void addAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format, const MeshAttribute attrFlag);
}