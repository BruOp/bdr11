#pragma once
#include "pch.h"


namespace bdr
{
    enum BufferFormat : uint8_t
    {
        UINT16 = 0,
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
        INVALID = UINT8_MAX
    };

    enum BufferUsage : uint8_t
    {
        Invalid = 0,
        Vertex = (1 << 0),
        Index = (1 << 1),
        ShaderReadable = (1 << 2),
        ComputeWritable = (1 << 3),
        CpuWritable = (1 << 4),

    };

    enum BufferType : uint8_t
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
        uint8_t srvType = 0;
        uint8_t uavType = 0;
        uint8_t format = INVALID;
        ID3D11Buffer* buffer = nullptr;
        ID3D11UnorderedAccessView* uav = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
    };

    struct BufferCreationInfo
    {
        uint32_t numElements = 0;
        uint32_t elementSize = 0;
        uint8_t usage = BufferUsage::Invalid;
        BufferFormat format = BufferFormat::INVALID;
        BufferType type = BufferType::Default;
    };

    DXGI_FORMAT mapFormatToDXGI(const BufferFormat bufferFormat)
    {
        switch (bufferFormat) {
        case UINT16:
            return DXGI_FORMAT_R16_UINT;
        case UINT32:
            return DXGI_FORMAT_R32_UINT;
        case UNORM8_2:
            return DXGI_FORMAT_R8G8_UNORM;
        case UNORM16_2:
            return DXGI_FORMAT_R16G16_UNORM;
        case FLOAT_2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case FLOAT_3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case UINT8_4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case UNORM8_4:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case UINT16_4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case UNORM16_4:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case FLOAT_4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case STRUCTURED:
            return DXGI_FORMAT_UNKNOWN;

        default:
            throw std::runtime_error("Invalid Format");
        }
    }

    uint64_t getByteSize(const BufferCreationInfo& createInfo)
    {
        switch (createInfo.format) {
        case UINT16:
        case UNORM8_2:
            return 2u;
        case UINT32:
        case UNORM16_2:
        case UINT8_4:
        case UNORM8_4:
            return 4u;
        case FLOAT_2:
        case UINT16_4:
        case UNORM16_4:
            return 8u;
        case FLOAT_3:
            return 12u;
        case FLOAT_4:
            return 16u;
        case STRUCTURED:
            return createInfo.elementSize;
        default:
            throw std::runtime_error("Invalid Format");
        }
    }

    GPUBuffer createBuffer(
        ID3D11Device* pDevice,
        const void* data,
        const BufferCreationInfo& createInfo
    )
    {
        const uint16_t usageFlags = createInfo.usage;
        GPUBuffer buffer{};
        buffer.numElements = createInfo.numElements;
        buffer.usage = createInfo.usage;
        buffer.format = createInfo.format;

        ASSERT(createInfo.format != STRUCTURED || createInfo.elementSize != 0,
            "Structured Buffer Creation must include element size!");
        ASSERT(!(usageFlags & BufferUsage::Vertex & BufferUsage::Index),
            "A buffer cannot be bound as both an index and vertex buffer");
        
uint32_t d3dBind = 0;
        
        if (usageFlags & BufferUsage::Vertex) {
            d3dBind = D3D11_BIND_VERTEX_BUFFER;
        }
        else if (usageFlags & BufferUsage::Index) {
            d3dBind = D3D11_BIND_INDEX_BUFFER;
        }
        if (usageFlags & BufferUsage::ShaderReadable) {
            d3dBind |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usageFlags & BufferUsage::ComputeWritable) {
            d3dBind |= D3D11_BIND_UNORDERED_ACCESS;
        }

        D3D11_USAGE d3dUsage = D3D11_USAGE_DEFAULT;

        if (usageFlags & BufferUsage::CpuWritable) {
            ASSERT(!(usageFlags & BufferUsage::ComputeWritable), "Cannot write using both Compute and CPU");
            d3dUsage = D3D11_USAGE_DYNAMIC;
        }
        // Create our buffer
        uint32_t bufferSize = getByteSize(createInfo) * createInfo.numElements;
        D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC();
        bufferDesc.ByteWidth = bufferSize;
        bufferDesc.Usage = d3dUsage;
        bufferDesc.BindFlags = d3dBind;
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = data;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        if (usageFlags & BufferUsage::ComputeWritable) {
            if (usageFlags & BufferUsage::Vertex) {
                bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
                bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements = bufferSize / 4u;
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
                buffer.uavType = BufferType::ByteAddressed;
            }
            else {
                ASSERT(
                    !(createInfo.type == BufferType::Structured) || (createInfo.format == BufferFormat::STRUCTURED),
                    "Structured buffers must have appropriate type and format"
                );
                uavDesc.Format = mapFormatToDXGI(createInfo.format);
                uavDesc.Buffer = D3D11_BUFFER_UAV{ 0, createInfo.numElements };
                buffer.uavType = createInfo.type;
            }
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        if (usageFlags & BufferUsage::ShaderReadable) {
            ASSERT(!(usageFlags & D3D11_BIND_VERTEX_BUFFER), "Cannot create SRV for vertex buffers!");

            bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            srvDesc.Format = mapFormatToDXGI(createInfo.format);
            srvDesc.Buffer.ElementOffset = 0;
            srvDesc.Buffer.NumElements = createInfo.numElements;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            buffer.srvType = BufferType::Typed;
        }

        // Create the buffer with the device.
        DX::ThrowIfFailed(pDevice->CreateBuffer(&bufferDesc, &initData, &buffer.buffer));

        if (usageFlags & BufferUsage::ComputeWritable) {
            DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(buffer.buffer, &uavDesc, &buffer.uav));
        }
        if (usageFlags & BufferUsage::ShaderReadable) {
            DX::ThrowIfFailed(pDevice->CreateShaderResourceView(buffer.buffer, &srvDesc, &buffer.srv));
        }

        return buffer;
    }

}

// Example usage:
// CreateIndexBuffer()