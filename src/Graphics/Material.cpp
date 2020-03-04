#include "pch.h"
#include <string>
#include <fstream>
#include "d3dcompiler.h"

#include "Renderer.h"
#include "Material.h"

namespace bdr
{
    void reset(Material& material)
    {
        /*material.vertexCB.reset();
        material.pixelCB.reset();*/
        if (material.vertexShader) {
            material.vertexShader->Release();
            material.vertexShader = nullptr;
        }
        if (material.pixelShader) {
            material.pixelShader->Release();
            material.pixelShader = nullptr;
        }
    }

    uint32_t MaterialManager::initMaterial(const std::wstring& vsFile, const std::wstring& psFile)
    {
        Material material{};

        std::vector<uint8_t> vsBlob = DX::ReadData(vsFile.c_str());
        std::vector<uint8_t> psBlob = DX::ReadData(psFile.c_str());

        DX::ThrowIfFailed(pDevice->CreateVertexShader(vsBlob.data(), vsBlob.size(), nullptr, &material.vertexShader));
        DX::ThrowIfFailed(pDevice->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, &material.pixelShader));

        /*material.vertexCB.init(pDevice, false);
        material.pixelCB.init(pDevice, false);*/

        materials.push_back(material);
        return materials.size() - 1;
    }

    uint32_t MaterialManager::initMaterial(ID3DBlob* vsBlob, ID3DBlob* psBlob)
    {
        Material material{};
        DX::ThrowIfFailed(pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &material.vertexShader));
        DX::ThrowIfFailed(pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &material.pixelShader));

        /*material.vertexCB.init(pDevice, false);
        material.pixelCB.init(pDevice, false);*/

        materials.push_back(material);
        return materials.size() - 1;
    }

    std::string readFile(const char* filePath)
    {
        std::ifstream shaderFile{ filePath };
        std::string code;
        std::string line;
        if (shaderFile.is_open()) {
            while (std::getline(shaderFile, line)) {
                code += line;
                code += '\n';
            }
        }
        else {
            ERROR("Could not open material file");
        }
        shaderFile.close();
        return code;
    }

    void compileShader(const char* code, const D3D_SHADER_MACRO* macros, ID3DBlob** vsBlob, ID3DBlob** psBlob)
    {
        ID3DBlob* error = nullptr;
        auto result = D3DCompile(
            code,
            strlen(code),
            nullptr,
            macros,
            nullptr,
            "VSMain", "vs_5_0",
            0, 0,
            vsBlob,
            &error
        );

        if (error) {
            HALT((char*)error->GetBufferPointer());
        }

        D3DCompile(
            code,
            strlen(code),
            nullptr,
            macros,
            nullptr,
            "PSMain", "ps_5_0",
            0, 0,
            psBlob,
            &error
        );

        if (error) {
            HALT((char*)error->GetBufferPointer());
        }

    }

    uint32_t MaterialManager::getMaterial(const MaterialType type, const uint16_t permutation) const
    {
        size_t N = materials.size();
        for (size_t i = 0; i < N; ++i) {
            const Material& material = materials[i];
            if (material.type == type && material.permutation == permutation) {
                return i;
            }
        }
        return UINT32_MAX;
    }

    uint32_t getOrCreatePBRMaterial(MaterialManager& materialManager, const uint16_t textureFlags)
    {
        uint32_t existingIdx = materialManager.getMaterial(MaterialType::PBR, textureFlags);
        if (existingIdx != UINT32_MAX) {
            return existingIdx;
        }

        constexpr char shaderFileName[] = "../src/Shaders/pbr_shader.hlsl";
        std::string code{ readFile(shaderFileName) };

        size_t numMacros = 0;
        D3D_SHADER_MACRO macros[16u]{};
        if (textureFlags & TextureFlags::ALBEDO) {
            macros[numMacros++] = D3D_SHADER_MACRO{ "ALBEDO_MAP" };
        }
        if (textureFlags & TextureFlags::METALLIC_ROUGHNESS) {
            macros[numMacros++] = D3D_SHADER_MACRO{ "METALLIC_ROUGHNESS_MAP" };
        }
        if (textureFlags & TextureFlags::NORMAL_MAP) {
            macros[numMacros++] = D3D_SHADER_MACRO{ "NORMAL_MAPPING" };
        }
        if (textureFlags & TextureFlags::OCCLUSION) {
            macros[numMacros++] = D3D_SHADER_MACRO{ "OCCLUSION_MAP" };
        }
        if (textureFlags & TextureFlags::EMISSIVE) {
            macros[numMacros++] = D3D_SHADER_MACRO{ "EMISSIVE_MAP" };
        }

        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        compileShader(code.c_str(), macros, &vsBlob, &psBlob);
        uint32_t idx = materialManager.initMaterial(vsBlob, psBlob);
        Material& material = materialManager[idx];
        material.type = MaterialType::PBR;
        material.attributeRequriements = MaterialAttributeRequirements[uint64_t(material.type)];
        material.permutation = textureFlags;
        return idx;
    }

    void reset(Shader& shader)
    {
        if (shader.vertexShader) {
            shader.vertexShader->Release();
            shader.vertexShader = nullptr;
        }
        if (shader.pixelShader) {
            shader.pixelShader->Release();
            shader.pixelShader = nullptr;
        }
        if (shader.computeShader) {
            shader.computeShader->Release();
            shader.computeShader = nullptr;
        }
    }

    uint32_t createShader(Renderer& renderer, const ShaderDesc& shaderDesc)
    {
        // TODO;
        auto idx = renderer.shaders.create();
        return idx;
    }
}