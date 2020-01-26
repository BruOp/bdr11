#include "pch.h"

#include <string>
#include <fstream>

#include "Renderer.h"
#include "GPUBuffer.h"
#include "d3dcompiler.h"



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

    uint32_t createMesh(Renderer& renderer, const MeshCreationInfo& meshCreationInfo)
    {
        return static_cast<uint32_t>(renderer.meshes.create());
    }

    uint32_t getOrCreateBasicMaterial(Renderer& renderer)
    {
        uint32_t existingIdx = renderer.materials.getMaterial(MaterialType::Basic, 0);
        if (existingIdx != UINT32_MAX) {
            return existingIdx;
        }

        constexpr char shaderFileName[] = "../src/Shaders/basic.hlsl";
        std::ifstream shaderFile{ shaderFileName };
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

        ID3DBlob* error = nullptr;
        ID3DBlob* vsBlob = nullptr;
        auto result = D3DCompile(
            code.c_str(),
            strlen(code.c_str()),
            nullptr,
            nullptr,
            nullptr,
            "VSMain", "vs_5_0",
            0, 0,
            &vsBlob,
            &error
        );

        if (error) {
            Utility::Print((char*)error->GetBufferPointer());
        }
        DX::ThrowIfFailed(result);

        ID3DBlob* psBlob = nullptr;
        DX::ThrowIfFailed(D3DCompile(
            code.c_str(),
            strlen(code.c_str()),
            nullptr,
            nullptr,
            nullptr,
            "PSMain", "ps_5_0",
            0, 0,
            &psBlob,
            nullptr
        ));

        uint32_t idx = renderer.materials.initMaterial(vsBlob, psBlob);
        Material& material = renderer.materials[idx];
        material.type = MaterialType::Basic;
        material.permutation = 0;
        return idx;
    }
}

void onWindowResize(int width, int height)
{
    renderer.hasWindowSizeChanged(width, height);
}

void onWindowMove()
{
    auto r = renderer.getOutputSize();
    renderer.hasWindowSizeChanged(r.right, r.bottom);
}
