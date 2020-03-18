#pragma once
#include "pch.h"

#include "Resources.h"

namespace bdr
{
    class Renderer;

    struct BufferView
    {
        GPUBufferHandle constantBuffer;
        uint16_t offset;
        uint16_t sizeInBytes;
    };

    struct RenderObject
    {
        PipelineHandle pipelineId;
        MeshHandle mesh;
        ResourceBinderHandle resourceBinderId;
        BufferView vsConstantData;
        BufferView psConstantData;
    };
    RESOURCE_HANDLE(RenderObjectHandle);

    RenderObjectHandle createRenderObject(Renderer& renderer);
    void assignMesh(Renderer& renderer, const RenderObjectHandle roId, const MeshHandle meshId);

    void bindTexture(
        Renderer& renderer,
        const RenderObjectHandle roId,
        const std::string& name,
        const TextureHandle textureHandle
    );

}