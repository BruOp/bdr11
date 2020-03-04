#include "pch.h"
#include "PipelineStateObject.h"
#include "Renderer.cpp"

namespace bdr
{
    // TODO: Move to a sensible place
    std::string readFile(const char* filePath);
    void compileShader(const char* code, const D3D_SHADER_MACRO* macros, ID3DBlob** vsBlob, ID3DBlob** psBlob);

    void reset(PipelineState& pso)
    {
        pso.blendState->Release();
        pso.depthStencilState->Release();
        pso.rasterizerState->Release();
        // InputLayout and Material are managed separately...
        // TODO: Consider whether Material and InputLayout should be managed separately
    }

    uint32_t createPipelineState(Renderer& renderer, const PipelineStateDesc& pipelineDesc)
    {
        return uint32_t();
    }
}
