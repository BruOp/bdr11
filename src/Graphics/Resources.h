#pragma once
#include "pch.h"

#include <unordered_map>
#include <string>
#include <functional>

#include "Core/Map.h"
#include "DXHelpers.h"
#include "RenderStates.h"

namespace bdr
{
    constexpr uint32_t kInvalidHandle = UINT32_MAX;

    // Copied from BGFX and others
#define RESOURCE_HANDLE(_name)      \
	struct _name { uint32_t idx = kInvalidHandle; }; \
    inline bool isValid(_name _handle) { return _handle.idx != bdr::kInvalidHandle; };

#define INVALID_HANDLE \
    { bdr::kInvalidHandle }

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
        NONE = 0,
        VERTEX_STAGE = 1 << 0,
        PIXEL_STAGE = 1 << 1,
        VERTEX_PIXEL_STAGES = VERTEX_STAGE | PIXEL_STAGE,
        COMPUTE_STAGE = 1 << 2,
    };

    enum struct BoundResourceType : uint8_t
    {
        INVALID = 0,
        CONSTANT_BUFFER,
        WRITABLE_BUFFER,
        READABLE_BUFFER,
        SAMPLER,
    };

    enum struct BindingLayoutUsage : uint8_t
    {
        PER_FRAME = 0,
        PER_VIEW,
        PER_DRAW,
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

    RESOURCE_HANDLE(MeshHandle);
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
        MeshHandle preskinMeshId = INVALID_HANDLE;
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

    struct InputLayoutDesc
    {
        uint8_t numAttributes = 0;
        MeshAttribute attributes[Mesh::maxAttrCount] = { MeshAttribute::INVALID };
        BufferFormat bufferFormats[Mesh::maxAttrCount] = { BufferFormat::INVALID };
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
    RESOURCE_HANDLE(TextureHandle);

    struct BoundResourceDesc
    {
        char name[64] = "";
        BoundResourceType type = BoundResourceType::INVALID;
        PipelineStage stages = PipelineStage::NONE;
    };

    struct ResourceBindingLayoutDesc
    {
        constexpr static size_t maxResources = 24;
        BoundResourceDesc resourceDescs[maxResources] = {};
    };

    // Warning: Not a POD
    struct ResourceBindingLayout
    {
        struct Slice
        {
            BoundResourceType resourceType = BoundResourceType::INVALID;
            uint32_t offset = UINT32_MAX;
        };

        uint8_t readableBufferCount = 0;
        uint8_t writableBufferCount = 0;
        uint8_t samplerCount = 0;
        // Maps our resources by name to their local offsets within the ResourceBindingHeap
        // Note that this is used to both allocate slots in our heap (returning a ResourceBinder) 
        // and to set resource pointers using an allocated ResourceBinder
        SimpleMap32<Slice> resourceMap;
    };

    struct ResourceBindingHeap
    {
        std::vector<ID3D11ShaderResourceView*> srvs;
        std::vector<ID3D11UnorderedAccessView*> uavs;
        std::vector<ID3D11SamplerState*> samplers;
    };

    struct ShaderMacro
    {
        char name[32] = "";
    };

    // Warning: not a POD
    struct PipelineStateDefinition
    {
        struct BindingMapView
        {
            uint32_t offset = UINT32_MAX;
            uint32_t count = 0;
        };

        PipelineStage stages;
        InputLayoutDesc inputLayoutDesc;
        DepthStencilDesc depthStencilState = {};
        RasterStateDesc rasterStateDesc = {};
        BlendStateDesc blendState = {};
        ResourceBindingLayoutDesc requiredResources = {};
        ShaderMacro macros[16] = { };
        ResourceBindingLayoutDesc optionalResources = {};
        // The optionalResourceMap maps our shader macros by name to entries in our optionalResources' array of ResourceBindingDescs
        // So an entry like { "NORMAL_MAPPING", { 4, 2 } } tells us that if the NORMAL_MAPPING macro is provided, we must include
        // the optional resources defined from optionalResources[4] to optionalResources[6] in the Pipeline's per draw ResourceBindingLayout.
        // It's kind of clumsy, I know.
        SimpleMap32<BindingMapView> optionalResourceMap = {};
        // Don't pass this in
        uint32_t shaderCodeId = UINT32_MAX;
    };
    struct PipelineStateDefinitionHandle { uint32_t idx; };

    // Warning: not a POD
    struct PipelineState
    {
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11ComputeShader* computeShader = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11DepthStencilState* depthStencilState = nullptr;
        ID3D11RasterizerState* rasterizerState = nullptr;
        ID3D11BlendState* blendState = nullptr;
        ResourceBindingLayout resourceLayout;
    };
    RESOURCE_HANDLE(PipelineHandle);

    // Used to track, update and bind the resources for a specific draw call.
    struct ResourceBinder
    {
        uint32_t readableBufferOffset;
        uint32_t writableBufferOffset;
        uint32_t samplerOffset;
        PipelineStage stage;
    };
    RESOURCE_HANDLE(ResourceBinderHandle);

    RESOURCE_HANDLE(GPUBufferHandle);
}