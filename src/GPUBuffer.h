#pragma once
#include "pch.h"


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
        Unused = 0,
        Vertex = (1 << 0),
        Index = (1 << 1),
        ShaderReadable = (1 << 2),
        ComputeWritable = (1 << 3),
        CpuWritable = (1 << 4),

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

        void reset();
    };

    struct BufferCreationInfo
    {
        // The number of elements in the buffer
        uint32_t numElements = 0;
        // elementSize is only used for structured buffers where the DXGI format is unknown
        uint32_t elementSize = 0;
        // How the buffer is going to be used -- e.g. does the CPU need write access? Check BufferUsage
        uint8_t usage = BufferUsage::Unused;
        // The layout of each element -- eg Float_3 or UINT16
        BufferFormat format = BufferFormat::INVALID;
        // Signals whether the buffer is structured, typed, byteAddress or just default (uses the format)
        BufferType type = BufferType::Default;
    };

    inline DXGI_FORMAT mapFormatToDXGI(const BufferFormat bufferFormat)
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

    inline uint32_t getByteSize(const BufferCreationInfo& createInfo)
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

    inline uint32_t getByteSize(const BufferFormat& format)
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

    GPUBuffer createBuffer(ID3D11Device* pDevice, const void* data, const BufferCreationInfo& createInfo);

}
