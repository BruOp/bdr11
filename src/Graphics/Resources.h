#pragma once
#include "pch.h"
#include <functional>
#include <unordered_map>

#include "DXHelpers.h"
#include "RenderStates.h"

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

    struct MeshDesc
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

    enum PipelineStage : uint8_t
    {
        VERTEX = 1 << 0,
        PIXEL = 1 << 1,
        VERTEX_PIXEL = PIXEL | VERTEX,
        COMPUTE = 1 << 2,
    };

    enum struct BoundResourceType : uint8_t
    {
        INVALID = 0,
        CONSTANT_BUFFER,
        WRITABLE_BUFFER,
        READABLE_BUFFER,
        SAMPLER,
    };

    struct BoundResourceDesc
    {
        std::string name;
        BoundResourceType type = BoundResourceType::INVALID;
        PipelineStage stages;
        uint8_t slot = UINT8_MAX;
    };

    enum struct BindingLayoutUsage : uint8_t
    {
        PER_FRAME = 0,
        PER_VIEW,
        PER_DRAW,
    };

    struct ResourceBindingLayoutDesc
    {
        constexpr static uint32_t maxResources = 16;
        BindingLayoutUsage usage = BindingLayoutUsage::PER_DRAW;
        BoundResourceDesc resourceDescs[maxResources] = {};
    };

    struct PipelineState
    {
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11ComputeShader* computeShader = nullptr;
        ID3D11InputLayout* inputLayout;
        ID3D11DepthStencilState* depthStencilState;
        ID3D11RasterizerState* rasterizerState;
        ID3D11BlendState* blendState;
        ResourceBindingLayout resourceBindingLayout;
    };

    struct PipelineStateDesc
    {
        std::string name;
        std::string file;
        PipelineStage stages;
        uint8_t meshAttributes;
        ResourceBindingLayoutDesc requiredResources;
        ResourceBindingLayoutDesc optionalResources;
        // TODO : Render targets/Outputs
        std::function<PipelineState(uint32_t)> creationFunc;
    };

    struct ResourceBindingObject
    {
        static constexpr size_t maxCountPerType = 16;

        uint32_t layoutId;
        ID3D11UnorderedAccessView* uavs[maxCountPerType]; // Should these be API specific or not?
        ID3D11ShaderResourceView* srvs[maxCountPerType];
        ID3D11SamplerState* samplers[maxCountPerType];
        ID3D11Buffer* CBHandles[maxCountPerType];
    };

    struct ResourceBindingHeap
    {
        ID3D11ShaderResourceView** srvs;
        ID3D11UnorderedAccessView** uavs;
        ID3D11Buffer** cbs;
    };

    struct ResourceBinder
    {
        uint16_t heapId;
        uint8_t count;
        PipelineStage stage;
        uint32_t offset;
    };

    struct ResourceView
    {
        uint16_t heapId;
        BoundResourceType resourceType;
        uint32_t offset;
    };

    struct ResourceBindingLayout
    {
        std::unordered_map<std::string, ResourceView> resourcesMap;
        // Key is HeapID, value is count required in that heap for binding;
        std::unordered_map<uint16_t, uint32_t> countsByHeap;
    };
}