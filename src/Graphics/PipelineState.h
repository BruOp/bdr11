#pragma once
#include "pch.h"
#include "Resources.h"
#include "Core/Array.h"


namespace bdr
{
    class Renderer;

    void reset(PipelineState& pipelineState);

    ResourceBindingLayoutDesc getPerDrawLayoutDesc(
        const PipelineStateDefinition& pipelineDefinition,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    );


    PipelineStateDefinitionHandle registerPipelineStateDefinition(
        Renderer& renderer,
        const std::string& filePath,
        PipelineStateDefinition&& pipelineDefinition
    );

    PipelineHandle getOrCreatePipelineState(
        Renderer& renderer,
        const PipelineStateDefinitionHandle& pipelineDefinitionHandle,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    );

    ResourceBinderHandle allocateResourceBinder(Renderer& renderer, const PipelineHandle pipelineId);

    class PipelineStateMap
    {
    public:
        PipelineStateMap() : map{} { };
        ~PipelineStateMap()
        {
            for (size_t i = 0; i < map.size; i++) {
                reset(map.values[i]);
            }
        }

        UNCOPIABLE(PipelineStateMap);
        UNMOVABLE(PipelineStateMap);

        PipelineHandle insert(const uint32_t key, const PipelineState& pipeline)
        {
            const size_t idx = map.size;
            map.fast_insert(key, pipeline);
            return { uint32_t(idx) };
        }

        uint32_t getHashKey(const ShaderMacro shaderMacros[], const size_t numMacros) const
        {
            char macrosCompoundKey[_countof(PipelineStateDefinition::macros) * _countof(ShaderMacro::name)] = "";
            for (size_t i = 0; i < numMacros; ++i) {
                if (shaderMacros[i].name[0] == '\0') break;
                strcat(macrosCompoundKey, shaderMacros[i].name);
            }
            return map.hashKey(macrosCompoundKey);
        }

        PipelineHandle getHandle(const uint32_t key) const
        {
            size_t index = map.get_index(key);
            if (index != UINT64_MAX) {
                if (index >= UINT32_MAX) {
                    DEBUGPRINT("Cannot create this many pipelines! What the hell are you doing");
                    abort();
                }
                return { uint32_t(index) };
            }
            else {
                return INVALID_HANDLE;
            }
        }

        PipelineState& operator[](const PipelineHandle handle)
        {
            return map.values[handle.idx];
        }

        const PipelineState& operator[](const PipelineHandle handle) const
        {
            return map.values[handle.idx];
        }

        SimpleMap32<PipelineState> map;
    };

}