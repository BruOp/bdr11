#include "pch.h"
#include <DDSTextureLoader.h>
#include "Texture.h"

using namespace DirectX;

namespace bdr
{
    void Texture::reset()
    { }


    Texture Texture::createFromFile(ID3D11Device* pDevice, const std::string& fileName, const TextureCreationInfo& createInfo)
    {
        BufferUsage usage = static_cast<BufferUsage>(createInfo.usage);
        uint32_t d3dBindFlags = 0;
        uint32_t cpuAccess = 0;
        D3D11_USAGE d3dUsage = D3D11_USAGE_IMMUTABLE;
        if (usage & BufferUsage::CpuWritable) {
            ASSERT(!(usage & BufferUsage::ComputeWritable), "Cannot write using both Compute and CPU");
            d3dUsage = D3D11_USAGE_DYNAMIC;
            cpuAccess = D3D11_CPU_ACCESS_WRITE;
        }
        else if (usage & BufferUsage::ComputeWritable) {
            d3dUsage = D3D11_USAGE_DEFAULT;
        }

        if (usage & BufferUsage::ShaderReadable) {
            d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (usage & BufferUsage::ComputeWritable) {
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

}