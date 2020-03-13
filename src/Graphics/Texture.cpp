#include "pch.h"

#include <DDSTextureLoader.h>
#include "Renderer.h"
#include "Texture.h"

using namespace DirectX;

namespace bdr
{
    void reset(Texture& texture)
    {
        if (texture.texture) {
            texture.texture->Release();
        }
        if (texture.uav) {
            texture.uav->Release();
        }
        if (texture.srv) {
            texture.srv->Release();
        }
        if (texture.sampler) {
            texture.sampler->Release();
        }
        texture = Texture{};
    }

    Texture createFromFile(ID3D11Device* pDevice, const std::string& fileName, const TextureCreationInfo& createInfo)
    {
        BufferUsage usage = static_cast<BufferUsage>(createInfo.usage);
        uint32_t d3dBindFlags = 0;
        uint32_t cpuAccess = 0;
        D3D11_USAGE d3dUsage = D3D11_USAGE_IMMUTABLE;
        if (usage & BufferUsage::CPU_WRITABLE) {
            ASSERT(!(usage & BufferUsage::COMPUTE_WRITABLE), "Cannot write using both Compute and CPU");
            d3dUsage = D3D11_USAGE_DYNAMIC;
            cpuAccess = D3D11_CPU_ACCESS_WRITE;
        }
        else if (usage & BufferUsage::COMPUTE_WRITABLE) {
            d3dUsage = D3D11_USAGE_DEFAULT;
        }

        if (usage & BufferUsage::SHADER_READABLE) {
            d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usage & BufferUsage::COMPUTE_WRITABLE) {
            d3dBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }

        std::wstring wfile{ fileName.begin(), fileName.end() };
        Texture texture{};
        DX::ThrowIfFailed(CreateDDSTextureFromFileEx(
            pDevice,
            wfile.c_str(),
            0,
            d3dUsage,
            d3dBindFlags,
            cpuAccess,
            0,
            false,
            &texture.texture,
            &texture.srv
        ));

        return texture;
    }

    uint32_t createTextureFromFile(Renderer& renderer, const std::string& filePath, const TextureCreationInfo& createInfo)
    {
        Texture texture{ createFromFile(renderer.getDevice(), filePath, createInfo) };
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        samplerDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
        samplerDesc.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        DX::ThrowIfFailed(renderer.getDevice()->CreateSamplerState(&samplerDesc, &texture.sampler));

        uint32_t idx = renderer.textures.size();
        renderer.textures.add(texture);
        return idx;
    }

}