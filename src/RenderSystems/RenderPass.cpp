
#include "pch.h"

#include "RenderPass.h"
#include "Game/Camera.h"
#include "Game/Scene.h"
#include "Graphics/Renderer.h"


namespace bdr
{
    RenderObjectHandle assignRenderObject(RenderSystem& renderSystem, const RenderObjectDesc renderObjectDesc)
    {
        RenderPass& renderPass = renderSystem.getPass(renderObjectDesc.passId);
        RenderObject renderObject = {
            renderObjectDesc.entityId,
            renderObjectDesc.meshId,
            renderObjectDesc.pipelineId,
        };
        renderObject.resourceBinder = allocateResourceBinder(*renderSystem.renderer, renderObject.pipelineId);
        return renderPass.renderObjectsManager.addRenderObject(renderObjectDesc.passId, renderObject);
    }

    RenderObject& getRenderObject(RenderSystem& renderSystem, const RenderObjectHandle renderObjectId)
    {
        RenderPassHandle passId = getRenderPassHandle(renderObjectId);
        RenderPass& pass = renderSystem.getPass(passId);
        return pass.renderObjectsManager.getRenderObject(renderObjectId);
    }

    void bindTexture(
        RenderSystem& renderSystem,
        RenderObjectHandle renderObjectId,
        const std::string& name,
        const TextureHandle textureHandle
    )
    {
        RenderObject& renderObject = getRenderObject(renderSystem, renderObjectId);
        Renderer* renderer = renderSystem.renderer;
        ResourceBinder& binder = renderObject.resourceBinder;
        ResourceBindingHeap& heap = renderer->bindingHeap;
        const ResourceBindingLayout& layout = renderer->pipelines[renderObject.pipelineId].resourceLayout;
        const Texture& texture = renderer->textures[textureHandle];

        ResourceBindingLayout::Slice& resourceView = layout.resourceMap.get(name + "_map");
        auto srvOffset = binder.readableBufferOffset + resourceView.offset;
        heap.srvs[srvOffset] = texture.srv;

        resourceView = layout.resourceMap.get(name + "_sampler");
        auto samplerOffset = binder.samplerOffset + resourceView.offset;
        heap.samplers[samplerOffset] = texture.sampler;
    }


    // TODO Add back skinning pass
    /*
    RenderPassHandle addSkinningPass(RenderSystem& renderSystem, View* view)
    {
        RenderPassHandle passId = renderSystem.createNewPass();
        RenderPass& pass = renderSystem.getPass(passId);
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
                    Mesh& mesh = renderer->meshes[renderObjects];
                    ASSERT(isValid(mesh.preskinMeshId), "Skinned meshes must have preskinned mesh");
                    Mesh& preskin = renderer->meshes[mesh.preskinMeshId];
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

        return passId;
    }
    */

    RenderPassHandle addMeshPass(RenderSystem& renderSystem, View* view)
    {
        RenderPassHandle passId = renderSystem.createNewPass();
        RenderPass& pass = renderSystem.getPass(passId);
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

            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            setConstants(renderer, view);
            const auto& renderObjectsAoA = pass.renderObjectsManager.renderObjectsAoA;
            for (size_t renderAoAIdx = 0; renderAoAIdx < renderObjectsAoA.size(); renderAoAIdx++) {
                const PipelineHandle pipelineId = pass.renderObjectsManager.pipelineHandles[renderAoAIdx];
                const PipelineState& pipelineState = renderer->pipelines[pipelineId];
                const ResourceBindingLayout& layout = pipelineState.resourceLayout;
                const ResourceBindingHeap& heap = renderer->bindingHeap;

                const std::vector<RenderObject>& renderObjectList = renderObjectsAoA[renderAoAIdx];

                // Set shaders
                context->VSSetShader(pipelineState.vertexShader, nullptr, 0);
                context->PSSetShader(pipelineState.pixelShader, nullptr, 0);

                context->OMSetDepthStencilState(pipelineState.depthStencilState, 0);
                context->OMSetBlendState(pipelineState.blendState, nullptr, 0xFF);
                context->RSSetState(pipelineState.rasterizerState);

                context->IASetInputLayout(pipelineState.inputLayout);

                for (size_t i = 0; i < renderObjectList.size(); ++i) {

                    const RenderObject& renderObject = renderObjectList[i];
                    const uint32_t entityId = renderObject.entityId;

                    const DrawConstants& drawConstants = registry.drawConstants[entityId];
                    const Mesh& mesh = renderer->meshes[renderObject.meshId];

                    ASSERT(mesh.inputLayoutHandle == pipelineState.inputLayout);
                    ID3D11Buffer* vbuffers[Mesh::maxAttrCount] = { nullptr };
                    collectBuffers(mesh, vbuffers);

                    context->IASetVertexBuffers(0, mesh.numPresentAttr, vbuffers, mesh.strides, offsets);
                    context->IASetIndexBuffer(mesh.indexBuffer.buffer, mapFormatToDXGI(mesh.indexBuffer.format), 0);

                    // Set constant buffers
                    vertexCB.copyToGPU(context, drawConstants);

                    // Set resources (textures, samplers)
                    if (hasResources(pipelineState)) {
                        const ResourceBinder& binder = renderObject.resourceBinder;

                        ID3D11ShaderResourceView* const* srvs = &heap.srvs[binder.readableBufferOffset];
                        ID3D11SamplerState* const* samplers = &heap.samplers[binder.samplerOffset];

                        context->PSSetShaderResources(0, layout.readableBufferCount, srvs);
                        context->PSSetSamplers(0, layout.samplerCount, samplers);
                    }

                    context->VSSetConstantBuffers(1, 1, &vertexCB.buffer);
                    context->DrawIndexed(mesh.numIndices, 0, 0);
                }
            }
        };
        pass.tearDown = [](Renderer* renderer) {
            renderer->getContext()->ClearState();
        };
        pass.destroy = [&]() {
            vertexCB.reset();
        };

        return passId;
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

    void RenderSystem::init(Renderer* _renderer)
    {
        renderer = _renderer;
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
