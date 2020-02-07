
#include "pch.h"

#include "RenderPass.h"
#include "Game/Camera.h"
#include "Game/Scene.h"
#include "Graphics/Renderer.h"


namespace bdr
{
    void addSkinningPass(RenderSystem& renderGraph, View* view)
    {
        RenderPass& pass = renderGraph.createNewPass();
        pass.name = L"Skinning Pass";
        pass.views.push_back(view);

        pass.renderView = [](Renderer* renderer, const View& view) {
            const Scene& scene = *view.scene;
            const ECSRegistry& registry = scene.registry;
            ID3D11DeviceContext* context = renderer->getContext();
            constexpr uint8_t meshAttrRequirements = 0u
                | MeshAttribute::POSITION
                | MeshAttribute::NORMAL
                | MeshAttribute::BLENDWEIGHT
                | MeshAttribute::BLENDINDICES;

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];

                if (cmpMask & CmpMasks::SKIN) {
                    // Compute joint matrices
                    const Skin& skin = scene.skins[registry.skinIds[entityId]];
                    Mesh& mesh = renderer->meshes[registry.meshes[entityId]];
                    ASSERT(mesh.preskinMeshIdx != UINT32_MAX, "Skinned meshes must have preskinned mesh");
                    Mesh& preskin = renderer->meshes[mesh.preskinMeshIdx];
                    GPUBuffer& jointBuffer = renderer->jointBuffers[registry.jointBuffer[entityId]];
                    std::vector<glm::mat4> jointMatrices(skin.inverseBindMatrices.size());
                    const glm::mat4 invModel{ glm::inverse(registry.globalMatrices[entityId]) };

                    for (size_t joint = 0; joint < jointMatrices.size(); ++joint) {
                        uint32_t jointEntity = skin.jointEntities[joint];
                        jointMatrices[joint] = skin.inverseBindMatrices[joint] * registry.globalMatrices[jointEntity] * invModel;
                    }

                    ASSERT(jointBuffer.buffer != nullptr);
                    D3D11_MAPPED_SUBRESOURCE mappedResource;
                    DX::ThrowIfFailed(context->Map(jointBuffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
                    CopyMemory(mappedResource.pData, jointMatrices.data(), sizeof(glm::mat4) * jointMatrices.size());
                    context->Unmap(jointBuffer.buffer, 0);

                    ID3D11ShaderResourceView* srvs[4u] = { nullptr };
                    collectViews(preskin, meshAttrRequirements, srvs);
                    ID3D11UnorderedAccessView* uavs[2u] = { nullptr };
                    collectViews(mesh, MeshAttribute::POSITION | MeshAttribute::NORMAL, uavs);

                    context->CSSetShaderResources(0u, 1u, &jointBuffer.srv);
                    context->CSSetShaderResources(1u, _countof(srvs), srvs);
                    context->CSSetUnorderedAccessViews(0u, _countof(uavs), uavs, nullptr);
                    context->CSSetShader(renderer->computeShader.Get(), nullptr, 0);
                    uint32_t numDispatches = uint32_t(ceil(float(mesh.numVertices) / 64.0f));
                    context->Dispatch(numDispatches, 1, 1);
                }
            }
        };

        pass.tearDown = [](Renderer* renderer) {
            ID3D11DeviceContext1* context = renderer->getContext();

            ID3D11UnorderedAccessView* nullUAVs[2u] = { nullptr };
            context->CSSetUnorderedAccessViews(0u, _countof(nullUAVs), nullUAVs, nullptr);
            ID3D11ShaderResourceView* nullSRVs[4u] = { nullptr };
            context->CSSetShaderResources(0u, _countof(nullSRVs), nullSRVs);
        };
    }


    void addBasicPass(RenderSystem& renderGraph, View* view)
    {
        RenderPass& pass = renderGraph.createNewPass();
        pass.name = L"Mesh Pass";
        pass.views.push_back(view);
        //GPUBuffer vertexCB{};
        static ConstantBuffer<DrawConstants> vertexCB{};

        pass.setup = [&](Renderer* renderer) {
            /*    BufferCreationInfo bufferCreateInfo{};
                bufferCreateInfo.format = BufferFormat::FLOAT_4;
                bufferCreateInfo.numElements = 16;
                bufferCreateInfo.type = BufferType::Default;
                bufferCreateInfo.usage = BufferUsage::Constant;
                vertexCB = createBuffer(renderer->getDevice(), nullptr, bufferCreateInfo);*/
            vertexCB.init(renderer->getDevice(), false);
        };

        pass.renderView = [&](Renderer* renderer, const View& view) {
            constexpr uint32_t offsets[Mesh::maxAttrCount] = { 0 };
            const Scene& scene = *view.scene;
            const ECSRegistry& registry = scene.registry;
            ASSERT(view.type == ViewType::Camera);
            ID3D11DeviceContext* context = renderer->getContext();
            const uint32_t requirements = CmpMasks::MESH | CmpMasks::MATERIAL;

            setConstants(renderer, view);

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];
                if ((cmpMask & requirements) == requirements) {
                    const DrawConstants& drawConstants = registry.drawConstants[entityId];
                    const Material& material = renderer->materials[registry.materials[entityId]];
                    const Mesh& mesh = renderer->meshes[registry.meshes[entityId]];

                    ID3D11Buffer* vbuffers[Mesh::maxAttrCount] = { nullptr };
                    collectBuffers(mesh, material.attributeRequriements, vbuffers);

                    // Set IAInputLayout
                    context->IASetVertexBuffers(0, mesh.numPresentAttr, vbuffers, mesh.strides, offsets);
                    context->IASetIndexBuffer(mesh.indexBuffer.buffer, mapFormatToDXGI(mesh.indexBuffer.format), 0);
                    context->IASetInputLayout(mesh.inputLayoutHandle);

                    // Set shaders
                    context->VSSetShader(material.vertexShader, nullptr, 0);
                    context->PSSetShader(material.pixelShader, nullptr, 0);

                    // Set constant buffers
                    vertexCB.copyToGPU(context, drawConstants);

                    // Set textures, if possible
                    if (cmpMask & CmpMasks::TEXTURED) {
                        const TextureSet& textureSet = registry.textures[entityId];

                        ID3D11ShaderResourceView* srvs[_countof(textureSet.textures)] = { nullptr };
                        ID3D11SamplerState* samplers[_countof(textureSet.textures)] = { nullptr };
                        for (uint16_t i = 0; i < textureSet.numTextures; i++) {
                            srvs[i] = renderer->textures[textureSet.textures[i]].srv;
                            samplers[i] = renderer->textures[textureSet.textures[i]].sampler;
                        }

                        context->PSSetShaderResources(0, textureSet.numTextures, srvs);
                        context->PSSetSamplers(0, textureSet.numTextures, samplers);
                    }

                    context->VSSetConstantBuffers(1, 1, &vertexCB.buffer);
                    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context->DrawIndexed(mesh.numIndices, 0, 0);
                }
            }
        };
        pass.tearDown = [](Renderer* renderer) {
            ID3D11Buffer* nullVB[Mesh::maxAttrCount] = { nullptr };
            uint32_t nullStrides[Mesh::maxAttrCount]{ 0 };
            uint32_t nullOffsets[Mesh::maxAttrCount]{ 0 };
            renderer->getContext()->IASetVertexBuffers(0, 5, nullVB, nullStrides, nullOffsets);
        };
        pass.destroy = [&]() {
            vertexCB.reset();
        };
    }

    void addPBRPass(RenderSystem& renderGraph, View* view)
    {
        RenderPass& pass = renderGraph.createNewPass();
        pass.name = L"Mesh Pass";
        pass.views.push_back(view);

        pass.renderView = [](Renderer* renderer, const View& view) {
            constexpr uint32_t offsets[Mesh::maxAttrCount] = { 0 };
            const Scene& scene = *view.scene;
            const ECSRegistry& registry = scene.registry;
            ASSERT(view.type == ViewType::Camera);
            ID3D11DeviceContext* context = renderer->getContext();
            constexpr uint8_t meshAttrRequirements = MeshAttribute::POSITION | MeshAttribute::NORMAL | MeshAttribute::TEXCOORD;

            setConstants(renderer, view);

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];
                const uint32_t requirements = CmpMasks::MESH | CmpMasks::MATERIAL;
                if ((cmpMask & requirements) == requirements) {
                    const DrawConstants& drawConstants = registry.drawConstants[entityId];
                    const Material& material = renderer->materials[registry.materials[entityId]];
                    const Mesh& mesh = renderer->meshes[registry.meshes[entityId]];
                    const TextureSet& textureSet = registry.textures[entityId];
                    const GenericMaterialData& materialData = registry.materialData[entityId];

                    ID3D11ShaderResourceView* srvs[_countof(textureSet.textures)] = { nullptr };
                    ID3D11SamplerState* samplers[_countof(textureSet.textures)] = { nullptr };
                    for (uint16_t i = 0; i < textureSet.numTextures; i++) {
                        srvs[i] = renderer->textures[textureSet.textures[i]].srv;
                        samplers[i] = renderer->textures[textureSet.textures[i]].sampler;
                    }

                    ID3D11Buffer* vbuffers[3u] = { nullptr };
                    collectBuffers(mesh, meshAttrRequirements, vbuffers);

                    // Set IAInputLayout
                    context->IASetVertexBuffers(0, mesh.numPresentAttr, vbuffers, mesh.strides, offsets);
                    context->IASetIndexBuffer(mesh.indexBuffer.buffer, mapFormatToDXGI(mesh.indexBuffer.format), 0);
                    context->IASetInputLayout(mesh.inputLayoutHandle);

                    // Set shaders
                    context->VSSetShader(material.vertexShader, nullptr, 0);
                    context->PSSetShader(material.pixelShader, nullptr, 0);

                    // Set constant buffers
                    /*material.vertexCB.copyToGPU(context, drawConstants);
                    material.pixelCB.copyToGPU(context, materialData);*/

                    //ID3D11Buffer* vsBuffers[] = { material.vertexCB };
                    //context->VSSetConstantBuffers(1, 1, vsBuffers);

                    /*context->PSSetConstantBuffers(1, 1, &material.pixelCB.buffer);*/
                    context->PSSetShaderResources(0, textureSet.numTextures, srvs);
                    context->PSSetSamplers(0, textureSet.numTextures, samplers);

                    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context->DrawIndexed(mesh.numIndices, 0, 0);
                }
            }
        };
        pass.tearDown = [](Renderer* renderer) {
            ID3D11Buffer* nullVB[Mesh::maxAttrCount] = { nullptr };
            uint32_t nullStrides[Mesh::maxAttrCount]{ 0 };
            uint32_t nullOffsets[Mesh::maxAttrCount]{ 0 };
            renderer->getContext()->IASetVertexBuffers(0, 5, nullVB, nullStrides, nullOffsets);
        };
    }

    RenderSystem::~RenderSystem()
    {
        for (RenderPass& renderPass : renderPasses) {
            if (renderPass.destroy) {
                renderPass.destroy();
            }
        }
        for (View& view : views) {
            view.viewCB.reset();
        }
    }

    void RenderSystem::run(Renderer* renderer) const
    {
        for (const RenderPass& renderPass : renderPasses) {
            renderer->deviceResources->PIXBeginEvent(renderPass.name.c_str());

            if (renderPass.views.size() > 0) {
                for (const View* view : renderPass.views) {
                    renderPass.renderView(renderer, *view);
                }
            }
            else {
                renderPass.render(renderer);
            }

            if (renderPass.tearDown) {
                renderPass.tearDown(renderer);
            }
            renderer->deviceResources->PIXEndEvent();
        }
    }

    void RenderSystem::init(Renderer* renderer)
    {
        ID3D11Device* device = renderer->getDevice();
        for (const RenderPass& renderPass : renderPasses) {
            if (renderPass.setup) {
                renderPass.setup(renderer);
            }
        }
        for (View& view : views) {
            view.viewCB.init(device);
        }
    }
}
