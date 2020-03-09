#pragma once
#include "pch.h"
#include "Resources.h"
#include "Core/Array.h"


namespace bdr
{
    class Renderer;

    void reset(PipelineState& pipelineState);

    void registerPipelineStateDefinition(Renderer& renderer, PipelineStateDefinition&& pipelineDefinition);

    uint32_t createPipelineState(Renderer& renderer, const std::string& pipelineName, const Array<ShaderMacro>& shaderMacros);
    //uint32_t createPipelineStateBuilder(Renderer& renderer, const PipelineStateBuilderDesc& desc);

}