#include "pch.h"
#include "GPUBuffer.h"
#include "Renderer.h"

namespace bdr
{
    void reset(GPUBuffer& gpuBuffer)
    {
        if (gpuBuffer.buffer) {
            gpuBuffer.buffer->Release();
        }
        if (gpuBuffer.srv) {
            gpuBuffer.srv->Release();
        }
        if (gpuBuffer.uav) {
            gpuBuffer.uav->Release();
        }
        gpuBuffer.numElements = 0;
        gpuBuffer = GPUBuffer{};
    }

    DXGI_FORMAT mapFormatToDXGI(const BufferFormat bufferFormat)
    {
        switch (bufferFormat) {
        case BufferFormat::UINT16:
            return DXGI_FORMAT_R16_UINT;
        case BufferFormat::UINT32:
            return DXGI_FORMAT_R32_UINT;
        case BufferFormat::UNORM8_2:
            return DXGI_FORMAT_R8G8_UNORM;
        case BufferFormat::UNORM16_2:
            return DXGI_FORMAT_R16G16_UNORM;
        case BufferFormat::FLOAT_2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case BufferFormat::FLOAT_3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case BufferFormat::UINT8_4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case BufferFormat::UNORM8_4:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case BufferFormat::UINT16_4:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case BufferFormat::UNORM16_4:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case BufferFormat::FLOAT_4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case BufferFormat::STRUCTURED:
            return DXGI_FORMAT_UNKNOWN;

        default:
            HALT("Invalid Format");
        }
    }

    uint32_t getByteSize(const BufferCreationInfo& createInfo)
    {
        switch (createInfo.format) {
        case BufferFormat::UINT16:
        case BufferFormat::UNORM8_2:
            return 2u;
        case BufferFormat::UINT32:
        case BufferFormat::UNORM16_2:
        case BufferFormat::UINT8_4:
        case BufferFormat::UNORM8_4:
            return 4u;
        case BufferFormat::FLOAT_2:
        case BufferFormat::UINT16_4:
        case BufferFormat::UNORM16_4:
            return 8u;
        case BufferFormat::FLOAT_3:
            return 12u;
        case BufferFormat::FLOAT_4:
            return 16u;
        case BufferFormat::STRUCTURED:
            return createInfo.elementSize;
        default:
            throw std::runtime_error("Invalid Format");
        }
    }

    uint32_t getByteSize(const BufferFormat& format)
    {
        switch (format) {
        case BufferFormat::UINT16:
        case BufferFormat::UNORM8_2:
            return 2u;
        case BufferFormat::UINT32:
        case BufferFormat::UNORM16_2:
        case BufferFormat::UINT8_4:
        case BufferFormat::UNORM8_4:
            return 4u;
        case BufferFormat::FLOAT_2:
        case BufferFormat::UINT16_4:
        case BufferFormat::UNORM16_4:
            return 8u;
        case BufferFormat::FLOAT_3:
            return 12u;
        case BufferFormat::FLOAT_4:
            return 16u;
        case BufferFormat::STRUCTURED:
            return 0u;
        default:
            throw std::runtime_error("Invalid Format");
        }
    }

    GPUBuffer createBuffer(ID3D11Device* pDevice, const void* data, const BufferCreationInfo& createInfo)
    {
        const uint16_t usageFlags = createInfo.usage;
        GPUBuffer buffer{};
        buffer.numElements = createInfo.numElements;
        buffer.usage = createInfo.usage;
        buffer.format = createInfo.format;

        ASSERT(createInfo.format != BufferFormat::STRUCTURED || createInfo.elementSize != 0,
            "Structured Buffer Creation must include element size!");
        ASSERT(!(usageFlags & BufferUsage::VERTEX & BufferUsage::INDEX),
            "A buffer cannot be bound as both an index and vertex buffer");

        // Create our buffer
        uint32_t bufferSize = getByteSize(createInfo) * createInfo.numElements;
        D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC();
        bufferDesc.ByteWidth = bufferSize;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = 0;

        if (usageFlags & BufferUsage::VERTEX) {
            bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
        }
        else if (usageFlags & BufferUsage::INDEX) {
            bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
        }
        if (usageFlags & BufferUsage::SHADER_READABLE) {
            bufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usageFlags & BufferUsage::COMPUTE_WRITABLE) {
            bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }
        if (usageFlags & BufferUsage::CONSTANT) {
            bufferDesc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
        }

        if (usageFlags & BufferUsage::CPU_WRITABLE) {
            ASSERT(!(usageFlags & BufferUsage::COMPUTE_WRITABLE), "Cannot write using both Compute and CPU");
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }

        if (createInfo.type == BufferType::Structured) {
            ASSERT(createInfo.format == BufferFormat::STRUCTURED,
                "Structured buffers must have appropriate type and format");
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            bufferDesc.StructureByteStride = getByteSize(createInfo);
        }

        D3D11_SUBRESOURCE_DATA initData;
        D3D11_SUBRESOURCE_DATA* pInitData = nullptr;
        if (data) {
            initData.pSysMem = data;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
            pInitData = &initData;
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        if (usageFlags & BufferUsage::COMPUTE_WRITABLE) {
            if (usageFlags & BufferUsage::VERTEX) {
                bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.FirstElement = 0;
                uavDesc.Buffer.NumElements = bufferSize / 4u;
                uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
                buffer.uavType = BufferType::ByteAddressed;
            }
            else {
                uavDesc.Format = mapFormatToDXGI(createInfo.format);
                uavDesc.Buffer = D3D11_BUFFER_UAV{ 0, createInfo.numElements };
                buffer.uavType = createInfo.type;
            }
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        if (usageFlags & BufferUsage::SHADER_READABLE) {
            ASSERT(!(usageFlags & D3D11_BIND_VERTEX_BUFFER), "Cannot create SRV for vertex buffers!");

            srvDesc.Format = mapFormatToDXGI(createInfo.format);
            srvDesc.Buffer.ElementOffset = 0;
            srvDesc.Buffer.NumElements = createInfo.numElements;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            buffer.srvType = createInfo.type;
        }

        // Create the buffer with the device.
        DX::ThrowIfFailed(pDevice->CreateBuffer(&bufferDesc, pInitData, &buffer.buffer));

        if (usageFlags & BufferUsage::COMPUTE_WRITABLE) {
            DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(buffer.buffer, &uavDesc, &buffer.uav));
        }
        if (usageFlags & BufferUsage::SHADER_READABLE) {
            DX::ThrowIfFailed(pDevice->CreateShaderResourceView(buffer.buffer, &srvDesc, &buffer.srv));
        }

        return buffer;
    }

    GPUBuffer createStructuredBuffer(ID3D11Device* device, const uint32_t elementSize, const uint32_t numElements)
    {
        BufferCreationInfo createInfo = {};
        createInfo.elementSize = elementSize;
        createInfo.format = BufferFormat::STRUCTURED;
        createInfo.numElements = numElements;
        createInfo.type = BufferType::Structured;
        createInfo.usage = BufferUsage::SHADER_READABLE | BufferUsage::CPU_WRITABLE;
        return createBuffer(device, nullptr, createInfo);
    }
}