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

                    context->CSSetShaderResources(0u, 1u, &jointBuffer.srv);
                    context->CSSetShaderResources(1u, 4u, preskin.srvs);
                    context->CSSetUnorderedAccessViews(0u, 2u, mesh.uavs, nullptr);
                    context->CSSetShader(renderer->computeShader.Get(), nullptr, 0);
                    uint32_t numDispatches = uint32_t(ceil(float(mesh.numVertices) / 64.0f));
                    context->Dispatch(numDispatches, 1, 1);
                }
            }
        };

        pass.tearDown = [](Renderer* renderer) {
            ID3D11DeviceContext1* context = renderer->getContext();

            ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr };
            context->CSSetUnorderedAccessViews(0u, 2u, nullUAVs, nullptr);
            ID3D11ShaderResourceView* nullSRVs[4] = { nullptr };
            context->CSSetShaderResources(0u, 4u, nullSRVs);
        };
    }

    void addBasicPass(RenderGraph& renderGraph, View* view)
    {
        RenderPass& pass = renderGraph.createNewPass();
        pass.name = L"Mesh Pass";
        pass.views.push_back(view);
        // TODO: Build Constant buffer data in batches
        //pass.setup = [](Renderer* renderer) {
        //};

        pass.renderView = [](Renderer* renderer, const View& view) {
            constexpr uint32_t offsets[bdr::Mesh::maxAttrCount] = { 0 };
            const Scene& scene = *view.scene;
            const bdr::ECSRegistry& registry = scene.registry;
            ASSERT(view.type & View::CameraType);
            ID3D11DeviceContext* context = renderer->getContext();

            setConstants(renderer, view);

            for (size_t entityId = 0; entityId < registry.numEntities; ++entityId) {
                const uint32_t cmpMask = registry.cmpMasks[entityId];
                const uint32_t requirements = bdr::CmpMasks::MESH | bdr::CmpMasks::MATERIAL;
                if ((cmpMask & requirements) == requirements) {
                    const bdr::DrawConstants& drawConstants = registry.drawConstants[entityId];
                    const bdr::Material& material = renderer->materials[registry.materials[entityId]];
                    const bdr::Mesh& mesh = renderer->meshes[registry.meshes[entityId]];

                    // Set IAInputLayout
                    context->IASetVertexBuffers(0, mesh.numPresentAttr, mesh.vertexBuffers, mesh.strides, offsets);
                    context->IASetIndexBuffer(mesh.indexBuffer, mesh.indexFormat, 0);
                    context->IASetInputLayout(renderer->inputLayoutManager[mesh.inputLayoutHandle]);

                    // Set shaders
                    context->VSSetShader(material.vertexShader, nullptr, 0);
                    context->PSSetShader(material.pixelShader, nullptr, 0);

                    // Set constant buffers
                    material.vertexCB.copyToGPU(context, drawConstants);

                    ID3D11Buffer* vsBuffers[] = { material.vertexCB };
                    context->VSSetConstantBuffers(1, 1, vsBuffers);

                    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context->DrawIndexed(mesh.numIndices, 0, 0);
                }
            }
        };
        pass.tearDown = [](Renderer* renderer) {
            ID3D11Buffer* nullVB[bdr::Mesh::maxAttrCount] = { nullptr };
            uint32_t nullStrides[bdr::Mesh::maxAttrCount]{ 0 };
            uint32_t nullOffsets[bdr::Mesh::maxAttrCount]{ 0 };
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
