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

    PipelineHandle createPipelineState(
        Renderer& renderer,
        const std::string& pipelineName,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    );

    ResourceBinderHandle allocateResourceBinder(Renderer& renderer, const PipelineHandle pipelineId);

}