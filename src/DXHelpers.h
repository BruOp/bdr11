#pragma once
#include "pch.h"

namespace bdr
{
    template<typename T>
    class ConstantBuffer
    {
    public:
        static_assert(sizeof(T) % 16 == 0, "Constant Buffer data structs must be 16 byte aligned");

        //T data;
        ID3D11Buffer* buffer;
        bool gpuWritable = false;

        void init(ID3D11Device* device, bool makeGpuWritable = false)
        {
            gpuWritable = makeGpuWritable;
            D3D11_BUFFER_DESC desc;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.MiscFlags = 0;
            desc.ByteWidth = static_cast<uint32_t>(sizeof(T) + (16 - (sizeof(T) % 16)));

            if (gpuWritable) {
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.CPUAccessFlags = 0;
            }
            else {
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            }
            if (buffer != nullptr) {
                buffer->Release();
            }
            DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, &buffer));
        }

        //inline void updateData(T& newData)
        //{
        //    data = newData;
        //}

        //inline void copyToGPU(ID3D11DeviceContext* deviceContext)
        //{
        //    ASSERT(buffer != nullptr);

        //    if (gpuWritable) {
        //        deviceContext->UpdateSubresource(buffer, 0, nullptr, &data, 0, 0);
        //    }
        //    else {
        //        D3D11_MAPPED_SUBRESOURCE mappedResource;
        //        DXCall(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        //        CopyMemory(mappedResource.pData, &data, sizeof(T));
        //        deviceContext->Unmap(Buffer, 0);
        //    }
        //}

        inline void copyToGPU(ID3D11DeviceContext* deviceContext, const T& newData) const
        {
            ASSERT(buffer != nullptr);

            if (gpuWritable) {
                deviceContext->UpdateSubresource(buffer, 0, nullptr, &newData, 0, 0);
            }
            else {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                DX::ThrowIfFailed(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
                CopyMemory(mappedResource.pData, &newData, sizeof(T));
                deviceContext->Unmap(buffer, 0);
            }
        }

        inline void reset()
        {
            if (buffer != nullptr) {
                buffer->Release();
                buffer = nullptr;
            }
        }

        inline operator ID3D11Buffer* () const
        {
            return buffer;
        }
    };
}