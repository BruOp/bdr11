#pragma once
#include "pch.h"
#include "DXHelpers.h"


namespace bdr
{
    enum class BufferFormat : uint8_t
    {
        INVALID = 0,
        UINT16,
        UINT32,
        UNORM8_2,
        UNORM16_2,
        FLOAT_2,
        FLOAT_3,
        UINT8_4,
        UNORM8_4,
        UINT16_4,
        UNORM16_4,
        FLOAT_4,

        STRUCTURED,
    };

    enum BufferUsage : uint8_t
    {
        UNUSED = 0,
        VERTEX = (1 << 0),
        INDEX = (1 << 1),
        SHADER_READABLE = (1 << 2),
        COMPUTE_WRITABLE = (1 << 3),
        CPU_WRITABLE = (1 << 4),
        CONSTANT = (1 << 5),

    };

    enum class BufferType : uint8_t
    {
        Default = 0,
        Typed,
        ByteAddressed,
        Structured,
    };


    struct GPUBuffer
    {
        uint32_t numElements = 0;
        uint8_t usage = 0;
        BufferType srvType = BufferType::Default;
        BufferType uavType = BufferType::Default;
        BufferFormat format = BufferFormat::INVALID;
        ID3D11Buffer* buffer = nullptr;
        ID3D11UnorderedAccessView* uav = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
    };

    struct BufferCreationInfo
    {
        // The number of elements in the buffer
        uint32_t numElements = 0;
        // elementSize is only used for structured buffers where the DXGI format is unknown
        uint32_t elementSize = 0;
        // How the buffer is going to be used -- e.g. does the CPU need write access? Check BufferUsage
        uint8_t usage = BufferUsage::UNUSED;
        // The layout of each element -- eg Float_3 or UINT16
        BufferFormat format = BufferFormat::INVALID;
        // Signals whether the buffer is structured, typed, byteAddress or just default (uses the format)
        BufferType type = BufferType::Default;
    };

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
    };

    struct MeshCreationInfo
    {
        uint8_t const* indexData = nullptr;
        BufferFormat indexFormat = BufferFormat::INVALID;
        uint8_t* data[Mesh::maxAttrCount] = { nullptr };
        BufferFormat bufferFormats[Mesh::maxAttrCount] = { BufferFormat::INVALID };
        MeshAttribute attributes[Mesh::maxAttrCount] = { MeshAttribute::INVALID };
        uint32_t strides[Mesh::maxAttrCount] = { 0 };
        uint8_t bufferUsages[Mesh::maxAttrCount] = { BufferUsage::UNUSED };
        uint32_t numIndices = 0;
        uint32_t numVertices = 0;
        uint8_t numAttributes = 0;
        uint8_t presentAttributesMask = 0;
    };

    enum class MaterialType : uint8_t
    {
        INVALID = 0,
        PBR = 1,
        BASIC = 2,
        CUSTOM = UINT8_MAX,
    };

    // Attribute requirements indexed by Material Type
    constexpr uint8_t MaterialAttributeRequirements[] = {
        MeshAttribute::INVALID,
        MeshAttribute::POSITION | MeshAttribute::NORMAL | MeshAttribute::TEXCOORD,
        MeshAttribute::POSITION | MeshAttribute::COLOR,
    };

    struct Material
    {
        MaterialType type = MaterialType::INVALID;
        uint16_t permutation = 0;
        uint8_t attributeRequriements = 0;
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
    };


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
    };

    enum TextureFlags : uint16_t
    {
        ALBEDO = (1 << 0),
        NORMAL_MAP = (1 << 1),
        METALLIC_ROUGHNESS = (1 << 2),
        OCCLUSION = (1 << 3),
        EMISSIVE = (1 << 4),
        DISABLED = (1 << 15),
        PBR_COMPATIBLE = ALBEDO | NORMAL_MAP | METALLIC_ROUGHNESS
    };
}