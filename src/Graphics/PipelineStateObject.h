#pragma once
#include "pch.h"

#include "RenderStates.h"
#include "Resources.h"

namespace bdr
{
    class Renderer;

    void reset(PipelineState& pso);
    uint32_t createPipelineState(Renderer& renderer, const PipelineStateDesc& pipelineDesc);
}