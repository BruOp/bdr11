#include "pch.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "d3dcompiler.h"

namespace bdr
{
    void reset(PipelineState& pipelineState)
    {
        release(pipelineState.vertexShader);
        release(pipelineState.pixelShader);
        release(pipelineState.computeShader);
        release(pipelineState.inputLayout);
        release(pipelineState.depthStencilState);
        release(pipelineState.rasterizerState);
        release(pipelineState.blendState);
    }

    void compileShader(const char* code, const D3D_SHADER_MACRO* macros, ID3DBlob** blob)
    {
        ID3DBlob* error = nullptr;
        auto result = D3DCompile(
            code,
            strlen(code),
            nullptr,
            macros,
            nullptr,
            "VSMain", "vs_5_0",
            0, 0,
            blob,
            &error
        );

        if (error) {
            HALT((char*)error->GetBufferPointer());
        }
    }

    void registerPipelineStateDefinition(Renderer& renderer, PipelineStateDefinition&& pipelineDefinition)
    {
        pipelineDefinition.shaderCode = readFile(pipelineDefinition.filePath.c_str());
        // TODO: This should fail in Release too
        bool insertionCompleted = renderer.pipelineDefinitions.insert(pipelineDefinition.name, pipelineDefinition);
        if (!insertionCompleted) {
            HALT("Insert Failed");
        }
    }

    uint32_t createPipelineState(Renderer& renderer, const std::string& pipelineName, const Array<ShaderMacro>& shaderMacros)
    {
        PipelineStateDefinition* pipelineDefinition;
        renderer.pipelineDefinitions.get(pipelineName, pipelineDefinition);

        uint32_t pipelineId = renderer.pipelines.create();

        PipelineState& pipeline = renderer.pipelines[pipelineId];
        D3D_SHADER_MACRO d3dMacros[16] = { nullptr };
        for (size_t i = 0; i < shaderMacros.size; i++) {
            d3dMacros[i] = D3D_SHADER_MACRO{ shaderMacros[i].name.c_str() };
        }

        if (pipelineDefinition->stages & PipelineStage::VERTEX_STAGE) {
            ID3DBlob* vsBlob;
            compileShader(pipelineDefinition->shaderCode.c_str(), d3dMacros, &vsBlob);
        }
    }
}