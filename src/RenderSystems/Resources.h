#pragma once
#include "pch.h"
#include "Graphics/Resources.h"

namespace bdr
{
    struct RenderPassHandle { uint8_t idx = UINT8_MAX; };
    struct RenderObjectHandle
    {
        // Composite id:
        //   - first 24 bits are the index into the RenderPass' objects array
        //   - last 8 bits are the passId
        uint32_t idx = UINT32_MAX;
        PipelineHandle pipelineId = INVALID_HANDLE;
    };

    struct RenderObjectDesc
    {
        uint32_t entityId = UINT32_MAX;
        RenderPassHandle passId = {};
        MeshHandle meshId = INVALID_HANDLE;
        PipelineHandle pipelineId = INVALID_HANDLE;
    };

    struct RenderObject
    {
        uint32_t entityId = UINT32_MAX;
        MeshHandle meshId = INVALID_HANDLE;
        PipelineHandle pipelineId = INVALID_HANDLE;
        ResourceBinder resourceBinder = {};
    };
}
