#pragma once
#include "pch.h"
#include "Resources.h"
#include "Core/Array.h"


namespace bdr
{
    class Renderer;

    void reset(PipelineState& pipelineState);

    void registerPipelineStateDefinition(
        Renderer& renderer,
        const std::string& name,
        const std::string& filePath,
        PipelineStateDefinition&& pipelineDefinition
    );

    uint32_t createPipelineState(
        Renderer& renderer,
        const std::string& pipelineName,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    );

    uint32_t allocateResourceBinder(Renderer& renderer, const uint32_t pipelineId);

}