#pragma once
#include "pch.h"
#include "Graphics/Resources.h"

namespace bdr
{
    struct RenderPassHandle { uint8_t idx = UINT8_MAX; };
    struct RenderObjectHandle
    {
        // Composite id:
        //   - first 24 bits are the index into the RenderPass' RenderObject AoA
        //   - last 8 bits are the passId
        uint32_t renderAoAIdxPassId = UINT32_MAX;
        uint32_t idx = UINT32_MAX;
    };

    inline RenderPassHandle getRenderPassHandle(const RenderObjectHandle renderObjectId)
    {
        return { uint8_t(renderObjectId.renderAoAIdxPassId) };
    }

    inline uint32_t getRenderAoAIdx(const RenderObjectHandle renderObjectId)
    {
        return renderObjectId.renderAoAIdxPassId >> 8;
    }

    inline uint32_t getRenderListIdx(const RenderObjectHandle renderObjectId)
    {
        return renderObjectId.idx;
    }

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
