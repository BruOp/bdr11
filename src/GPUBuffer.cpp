#include "pch.h"
#include "GPUBuffer.h"

namespace bdr
{
    GPUBuffer createBuffer(ID3D11Device* pDevice, const void* data, const BufferCreationInfo& createInfo)
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

        // Create our buffer
        uint32_t bufferSize = getByteSize(createInfo) * createInfo.numElements;
        D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC();
        bufferDesc.ByteWidth = bufferSize;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = 0;

        if (usageFlags & BufferUsage::Vertex) {
            bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
        }
        else if (usageFlags & BufferUsage::Index) {
            bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
        }
        if (usageFlags & BufferUsage::ShaderReadable) {
            bufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usageFlags & BufferUsage::ComputeWritable) {
            bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }

        if (usageFlags & BufferUsage::CpuWritable) {
            ASSERT(!(usageFlags & BufferUsage::ComputeWritable), "Cannot write using both Compute and CPU");
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
        if (usageFlags & BufferUsage::ComputeWritable) {
            if (usageFlags & BufferUsage::Vertex) {
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
        if (usageFlags & BufferUsage::ShaderReadable) {
            ASSERT(!(usageFlags & D3D11_BIND_VERTEX_BUFFER), "Cannot create SRV for vertex buffers!");

            srvDesc.Format = mapFormatToDXGI(createInfo.format);
            srvDesc.Buffer.ElementOffset = 0;
            srvDesc.Buffer.NumElements = createInfo.numElements;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            buffer.srvType = createInfo.type;
        }

        // Create the buffer with the device.
        DX::ThrowIfFailed(pDevice->CreateBuffer(&bufferDesc, pInitData, &buffer.buffer));

        if (usageFlags & BufferUsage::ComputeWritable) {
            DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(buffer.buffer, &uavDesc, &buffer.uav));
        }
        if (usageFlags & BufferUsage::ShaderReadable) {
            DX::ThrowIfFailed(pDevice->CreateShaderResourceView(buffer.buffer, &srvDesc, &buffer.srv));
        }

        return buffer;
    }
}