#include "pch.h"
#include "Renderer.h"

namespace bdr
{
    JointBuffer bdr::createJointBuffer(ID3D11Device* device, const Skin& skin)
    {
        JointBuffer jointBuffer{};

        D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(
            sizeof(DirectX::SimpleMath::Matrix) * skin.inverseBindMatrices.size(),
            D3D11_BIND_SHADER_RESOURCE,
            D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE,
            D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
            sizeof(DirectX::SimpleMath::Matrix)
        );
        // Create the buffer with the device.
        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, &jointBuffer.buffer));

        D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.Buffer = D3D11_BUFFER_SRV{ 0, uint32_t(skin.inverseBindMatrices.size()) };
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        DX::ThrowIfFailed(device->CreateShaderResourceView(jointBuffer.buffer, &desc, &jointBuffer.srv));
        return jointBuffer;
    }
}
