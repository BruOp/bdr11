#include "pch.h"
#include "Material.h"

namespace bdr
{
    void reset(Material& material)
    {
        //material.psoPerPass
    };


    uint32_t createMaterial(RenderPassManager& renderPassManager, const MaterialDesc& materialDesc)
    {
        // Create a layout
        ResourceBindingLayout layout{};
        for (const auto& resourceDesc : materialDesc.layoutDesc.resourceDescs) {
            uint32_t renderPass = getRenderPass(renderPassManager, resourceDesc.passName);
            uint32_t heapId = getBindingHeap(renderPass);
            HeapView& heapView = layout.resourceMap[resourceDesc.name];
            heapView.heapId = heapId;
            heapView.resourceType = materialDesc.resourceType;
            heapView.offset = ++layout.countsByHeap[heapId];
        }

        // Figure out which passes the material is a part of and store it in the mask
        //uint32_t passMask = TODO;
        for (const auto& passDesc : materialDesc.features) {
            passDesc.passName
        }

        //Material material{ layout, passMask, pipelines }
        // Add passIds
    };

    MaterialInstance createMaterialInstance(RenderPassManager& renderPassManager, const uint32_t materialId)
    {
        MaterialInstance materialInstance{ materialId };
        materialinstance
    };
}