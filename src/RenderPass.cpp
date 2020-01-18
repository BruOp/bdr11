
#include "pch.h"

#include "RenderPass.h"
#include "Camera.h"
#include "Scene.h"
#include "Renderer.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    void addSkinningPass(RenderGraph& renderGraph, View* view)
    {
        RenderPass& pass = renderGraph.createNewPass();
        pass.name = L"Skinning Pass";
        pass.views.push_back(view);

        pass.renderView = [](Renderer* renderer, const View& view) {
            const Scene& scene = *view.scene;
            const ECSRegistry& registry = scene.registry;
            ID3D11DeviceContext* context = renderer->getContext();
            constexpr uint8_t meshAttrRequirements = 0u
                | MeshAttributes::POSITION
                | MeshAttributes::NORMAL
                | MeshAttributes::BLENDWEIGHT
                | MeshAttributes::BLENDINDICES;

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];

                if (cmpMask & CmpMasks::SKIN) {
                    // Compute joint matrices
                    const Skin& skin = scene.skins[registry.skinIds[entityId]];
                    Mesh& mesh = renderer->meshes[registry.meshes[entityId]];
                    ASSERT(mesh.preskinMeshIdx != UINT32_MAX, "Skinned meshes must have preskinned mesh");
                    Mesh& preskin = renderer->meshes[mesh.preskinMeshIdx];
                    GPUBuffer& jointBuffer = renderer->jointBuffers[registry.jointBuffer[entityId]];
                    std::vector<Matrix> jointMatrices(skin.inverseBindMatrices.size());
                    const Matrix invModel{ registry.globalMatrices[entityId].Invert() };

                    for (size_t joint = 0; joint < jointMatrices.size(); ++joint) {
                        uint32_t jointEntity = skin.jointEntities[joint];
                        jointMatrices[joint] = (skin.inverseBindMatrices[joint] * registry.globalMatrices[jointEntity] * invModel).Transpose();
                    }

                    ASSERT(jointBuffer.buffer != nullptr);
                    D3D11_MAPPED_SUBRESOURCE mappedResource;
                    DX::ThrowIfFailed(context->Map(jointBuffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
                    CopyMemory(mappedResource.pData, jointMatrices.data(), sizeof(Matrix) * jointMatrices.size());
                    context->Unmap(jointBuffer.buffer, 0);

                    ID3D11ShaderResourceView* srvs[4u] = { nullptr };
                    collectBuffers(preskin, meshAttrRequirements, srvs);
                    ID3D11UnorderedAccessView* uavs[2u] = { nullptr };
                    collectBuffers(mesh, MeshAttributes::POSITION | MeshAttributes::NORMAL, uavs);

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

    void addBasicPass(RenderGraph& renderGraph, View* view)
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
            constexpr uint8_t meshAttrRequirements = MeshAttributes::POSITION | MeshAttributes::NORMAL | MeshAttributes::TEXCOORD;

            setConstants(renderer, view);

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];
                const uint32_t requirements = CmpMasks::MESH | CmpMasks::MATERIAL;
                if ((cmpMask & requirements) == requirements) {
                    const DrawConstants& drawConstants = registry.drawConstants[entityId];
                    const Material& material = renderer->materials[registry.materials[entityId]];
                    const Mesh& mesh = renderer->meshes[registry.meshes[entityId]];
                    const TextureSet& textureSet = registry.textures[entityId];

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
                    context->IASetInputLayout(renderer->inputLayoutManager[mesh.inputLayoutHandle]);

                    // Set shaders
                    context->VSSetShader(material.vertexShader, nullptr, 0);
                    context->PSSetShader(material.pixelShader, nullptr, 0);

                    // Set constant buffers
                    material.vertexCB.copyToGPU(context, drawConstants);

                    ID3D11Buffer* vsBuffers[] = { material.vertexCB };
                    context->VSSetConstantBuffers(1, 1, vsBuffers);

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

    void RenderGraph::run(Renderer* renderer) const
    {
        for (const RenderPass& renderPass : renderPasses) {
            renderer->deviceResources->PIXBeginEvent(renderPass.name.c_str());

            if (renderPass.setup) {
                renderPass.setup(renderer);
            }

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
}
