#include "pch.h"

#include <string>
#include <fstream>

#include "Renderer.h"
#include "GPUBuffer.h"
#include "d3dcompiler.h"


namespace bdr
{
    GPUBuffer createStructuredBuffer(ID3D11Device* device, const uint32_t elementSize, const uint32_t numElements)
    {
        BufferCreationInfo createInfo = {};
        createInfo.elementSize = elementSize;
        createInfo.format = BufferFormat::STRUCTURED;
        createInfo.numElements = numElements;
        createInfo.type = BufferType::Structured;
        createInfo.usage = BufferUsage::SHADER_READABLE | BufferUsage::CPU_WRITABLE;
        return createBuffer(device, nullptr, createInfo);
    }

    uint32_t createMesh(Renderer& renderer, const MeshDesc& meshCreateInfo)
    {
        uint32_t meshId = static_cast<uint32_t>(renderer.meshes.create());
        Mesh& mesh = renderer.meshes[meshId];

        mesh.numIndices = meshCreateInfo.numIndices;
        mesh.numVertices = meshCreateInfo.numVertices;

        BufferCreationInfo indexCreateInfo{};
        indexCreateInfo.numElements = meshCreateInfo.numIndices;
        indexCreateInfo.usage = BufferUsage::INDEX;
        indexCreateInfo.format = meshCreateInfo.indexFormat;

        ID3D11Device* device = renderer.getDevice();
        mesh.indexBuffer = createBuffer(device, meshCreateInfo.indexData, indexCreateInfo);

        for (size_t i = 0; i < meshCreateInfo.numAttributes; ++i) {
            BufferCreationInfo createInfo{};
            createInfo.numElements = meshCreateInfo.numVertices;
            createInfo.usage = meshCreateInfo.bufferUsages[i];
            createInfo.format = meshCreateInfo.bufferFormats[i];
            createInfo.type = BufferType::Default;

            if (createInfo.usage == BufferUsage::UNUSED) {
                continue;
            }

            mesh.vertexBuffers[i] = createBuffer(device, meshCreateInfo.data[i], createInfo);
            mesh.attributes[i] = meshCreateInfo.attributes[i];
            mesh.presentAttributesMask |= meshCreateInfo.attributes[i];
            mesh.strides[i] = meshCreateInfo.strides[i];
        }
        mesh.numPresentAttr = meshCreateInfo.numAttributes;
        mesh.inputLayoutHandle = renderer.inputLayoutManager.getOrCreateInputLayout(meshCreateInfo);
        return meshId;
    }

    uint32_t getOrCreateBasicMaterial(Renderer& renderer)
    {
        uint32_t existingIdx = renderer.materials.getMaterial(MaterialType::BASIC, 0);
        if (existingIdx != UINT32_MAX) {
            return existingIdx;
        }

        constexpr char shaderFileName[] = "../src/Shaders/basic.hlsl";
        std::string code{ readFile(shaderFileName) };
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        compileShader(code.c_str(), nullptr, &vsBlob, &psBlob);

        uint32_t idx = renderer.materials.initMaterial(vsBlob, psBlob);
        Material& material = renderer.materials[idx];
        material.type = MaterialType::BASIC;
        material.attributeRequriements = MaterialAttributeRequirements[uint64_t(material.type)];
        material.permutation = 0;
        return idx;
    }

    uint32_t createCustomMaterial(Renderer& renderer, const std::string& shaderPath, uint8_t attrRequirements)
    {
        ID3D11Device* device = renderer.getDevice();
        std::string code{ readFile(shaderPath.c_str()) };
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        compileShader(code.c_str(), nullptr, &vsBlob, &psBlob);
        uint32_t idx = renderer.materials.initMaterial(vsBlob, psBlob);
        // TODO: Figure out a better way to track materials. Perhaps a second index?
        Material& material = renderer.materials[idx];
        material.type = MaterialType::CUSTOM;
        material.attributeRequriements = attrRequirements;
        material.permutation = uint16_t(renderer.materials.size());
        return idx;
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

void onWindowResize(bdr::Renderer& renderer, int width, int height)
{
    renderer.hasWindowSizeChanged(width, height);
}

void onWindowMove(bdr::Renderer& renderer)
{
    RECT r = renderer.getOutputSize();
    renderer.hasWindowSizeChanged(r.right, r.bottom);
}
