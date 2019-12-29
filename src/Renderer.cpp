#include "pch.h"
#include "Renderer.h"
#include "GPUBuffer.h"


namespace bdr
{
    GPUBuffer bdr::createJointBuffer(ID3D11Device* device, const Skin& skin)
    {
        BufferCreationInfo createInfo = {};
        createInfo.elementSize = sizeof(DirectX::SimpleMath::Matrix);
        createInfo.format = BufferFormat::STRUCTURED;
        createInfo.numElements = skin.inverseBindMatrices.size();
        createInfo.type = BufferType::Structured;
        createInfo.usage = BufferUsage::ShaderReadable | BufferUsage::CpuWritable;
        return createBuffer(device, nullptr, createInfo);
    }
}
