#pragma once
#include "Core/Array.h"
#include "Resources.h"
#include "Renderer.h"

namespace bdr
{
    constexpr size_t kMaxNumPasses = 16;

    struct RenderPass;
    RESOURCE_HANDLE(RenderPassHandle);

    struct MaterialDesc
    {
        struct MaterialPassDesc
        {
            RenderPassHandle passId = INVALID_HANDLE;
            PipelineStateDefinitionHandle pipelineDefinitionId = INVALID_HANDLE;
            ShaderMacro macros[16] = { };
        };

        MaterialPassDesc materialPasses[kMaxNumPasses] = {};
    };

    struct Material
    {
        uint32_t passMask = 0;
        // Used to enable updating resources by name using just the material.
        // Might be unnecessary.
        // Specifically for per draw resource bindings, not view or frame ones.
        // Pass should take care of those automatically.
        SimpleMap32<PipelineHandle> resourcePipelineMap;
        // Indexed using the RenderPassHandle
        PipelineHandle pipelineIds[kMaxNumPasses];
    };
    RESOURCE_HANDLE(MaterialHandle);

    class RenderPassManager
    {
        std::vector<Material> materials;
    };

    Material createMaterial(const Renderer&, const RenderPassManager&, const MaterialDesc&);
    PipelineHandle getPipelineId(const Material& material, const RenderPassHandle passId);
}

