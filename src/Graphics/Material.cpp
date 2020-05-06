#include "pch.h"
#include "Material.h"

namespace bdr
{
    Material createMaterial(Renderer& renderer, const RenderPassManager& renderPassManager, const MaterialDesc& materialDesc)
    {
        Material material{};
        for (size_t i = 0; i < _countof(materialDesc.materialPasses); i++) {
            const MaterialDesc::MaterialPassDesc& matPassDesc = materialDesc.materialPasses[i];
            material.passMask |= 1 << matPassDesc.passId.idx;

            size_t numMacros = 0;
            for (; numMacros < _countof(matPassDesc.macros); ++numMacros) {
                if (matPassDesc.macros[numMacros].name[0] == '\0') break;
            }

            // Use the pipelineDefinitionID instead of the name
            PipelineHandle& pipelineId = getOrCreatePipelineState(renderer, matPassDesc.pipelineDefinitionId, matPassDesc.macros, numMacros);

            material.pipelineIds[matPassDesc.passId.idx] = pipelineId;

            const PipelineState& pipeline = renderer.pipelines[pipelineId];

            const PipelineStateDefinition& pipelineDefinition = renderer.pipelineDefinitions[matPassDesc.pipelineDefinitionId.idx];
            ResourceBindingLayoutDesc perDrawLayoutDesc = getPerDrawLayoutDesc(pipelineDefinition, matPassDesc.macros, numMacros);
            for (size_t i = 0; i < ResourceBindingLayoutDesc::maxResources; ++i) {
                const BoundResourceDesc& resourceDesc = perDrawLayoutDesc.resourceDescs[i];

                if (resourceDesc.type == BoundResourceType::INVALID) {
                    break;
                }

                bool inserted = material.resourcePipelineMap.insert(resourceDesc.name, pipelineId);
                ASSERT(inserted, "We're squashing uniforms! TODO: Allow non-unique uniform names for each pipeline");
            }
        }

        return material;
    };

    inline PipelineHandle getPipelineId(const Material& material, const RenderPassHandle passId)
    {
        return material.pipelineIds[passId.idx];
    }
}