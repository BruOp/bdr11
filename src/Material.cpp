#include "pch.h"
#include "Material.h"

namespace bdr
{
    void Material::reset()
    {
        vertexCB.reset();
        pixelCB.reset();
        if (vertexShader) {
            vertexShader->Release();
            vertexShader = nullptr;
        }
        if (pixelShader) {
            pixelShader->Release();
            pixelShader = nullptr;
        }
    }

    uint32_t MaterialManager::initMaterial(ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile)
    {
        Material material{};

        std::vector<uint8_t> vsBlob = DX::ReadData(vsFile.c_str());
        std::vector<uint8_t> psBlob = DX::ReadData(psFile.c_str());

        DX::ThrowIfFailed(device->CreateVertexShader(vsBlob.data(), vsBlob.size(), nullptr, &material.vertexShader));
        DX::ThrowIfFailed(device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, &material.pixelShader));
        
        material.vertexCB.init(device, false);
        material.pixelCB.init(device, false);
        
        materials.push_back(material);
        return materials.size() - 1;
    }
}