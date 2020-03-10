#include "pch.h"
#include "PipelineState.h"
#include "Renderer.h"
#include "d3dcompiler.h"

namespace bdr
{
    D3D11_COMPARISON_FUNC getD3DComparisonFunc(const ComparisonFunc comparisonFunc)
    {
        switch (comparisonFunc) {
        case ComparisonFunc::LESS:
            return D3D11_COMPARISON_LESS;
        case ComparisonFunc::LESS_OR_EQUAL:
            return D3D11_COMPARISON_LESS_EQUAL;
        case ComparisonFunc::EQUAL:
            return D3D11_COMPARISON_EQUAL;
        case ComparisonFunc::GREATER_OR_EQUAL:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case ComparisonFunc::GREATER:
            return D3D11_COMPARISON_GREATER;
        case ComparisonFunc::ALWAYS:
            return D3D11_COMPARISON_ALWAYS;
        case ComparisonFunc::NEVER:
        default:
            return D3D11_COMPARISON_NEVER;
        }
    }

    D3D11_STENCIL_OP getD3DStencilOp(const StencilOp stencilOp)
    {
        switch (stencilOp) {
        case StencilOp::KEEP:
            return D3D11_STENCIL_OP_KEEP;
        case StencilOp::ZERO:
            return D3D11_STENCIL_OP_ZERO;
        case StencilOp::REPLACE:
            return D3D11_STENCIL_OP_REPLACE;
        case StencilOp::INCR_SAT:
            return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOp::DECR_SAT:
            return D3D11_STENCIL_OP_DECR_SAT;
        case StencilOp::INVERT:
            return D3D11_STENCIL_OP_INVERT;
        case StencilOp::INCR:
            return D3D11_STENCIL_OP_INCR;
        case StencilOp::DECR:
            return D3D11_STENCIL_OP_DECR;
        default:
            return D3D11_STENCIL_OP_ZERO;
        }
    }

    D3D11_FILL_MODE getD3DFillMode(const FillMode mode)
    {
        switch (mode) {
        case FillMode::WIREFRAME:
            return D3D11_FILL_WIREFRAME;
        case FillMode::SOLID:
        default:
            return D3D11_FILL_SOLID;
        }
    }

    D3D11_DEPTH_STENCILOP_DESC getD3DStencilOpDesc(const StencilFuncDesc& funcDesc)
    {
        return D3D11_DEPTH_STENCILOP_DESC{
            getD3DStencilOp(funcDesc.onStencilFail),
            getD3DStencilOp(funcDesc.onStencilPassDepthFail),
            getD3DStencilOp(funcDesc.onStencilPassDepthPass),
            getD3DComparisonFunc(funcDesc.comparisonFunc),
        };
    }

    D3D11_RASTERIZER_DESC toD3D11RasterizerDesc(const RasterStateDesc& rasterDesc)
    {
        D3D11_RASTERIZER_DESC desc;
        desc.FillMode = getD3DFillMode(rasterDesc.fillMode);

        switch (rasterDesc.cullMode) {
        case CullMode::BACK_CCW:
            desc.CullMode = D3D11_CULL_BACK;
            desc.FrontCounterClockwise = true;
            break;
        case CullMode::BACK_CW:
            desc.CullMode = D3D11_CULL_BACK;
            desc.FrontCounterClockwise = false;
            break;
        case CullMode::FRONT_CCW:
            desc.CullMode = D3D11_CULL_FRONT;
            desc.FrontCounterClockwise = true;
            break;
        case CullMode::FRONT_CW:
            desc.CullMode = D3D11_CULL_FRONT;
            desc.FrontCounterClockwise = false;
            break;
        case CullMode::NONE:
        default:
            desc.CullMode = D3D11_CULL_NONE;
            desc.FrontCounterClockwise = false;
            break;
        }

        desc.DepthBias = rasterDesc.depthBias;
        desc.DepthBiasClamp = rasterDesc.depthBiasClamp;
        desc.SlopeScaledDepthBias = rasterDesc.slopeScaledDepthBias;

        if (rasterDesc.flags & RasterizerFlags::DEPTH_CLIP_ENABLED) {
            desc.DepthClipEnable = true;
        }
        if (rasterDesc.flags & RasterizerFlags::ANTIALIASED_LINES) {
            desc.AntialiasedLineEnable = true;
        }
        if (rasterDesc.flags & RasterizerFlags::MULTISAMPLE) {
            desc.MultisampleEnable = true;
        }
        if (rasterDesc.flags & RasterizerFlags::SCISSOR_ENABLED) {
            desc.ScissorEnable = true;
        }

        return desc;
    }


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

    ResourceBindingLayout createLayout(Renderer& renderer, const ResourceBindingLayoutDesc& layoutDesc)
    {
        ResourceBindingLayout layout{};

        for (const auto& resourceDesc : layoutDesc.resourceDescs) {
            if (resourceDesc.type == BoundResourceType::CONSTANT_BUFFER) {
                // TODO
                HALT("This resource type is not supported!");
            }
            else if (resourceDesc.type == BoundResourceType::READABLE_BUFFER) {
                layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.readableBufferCount++ });
            }
            else if (resourceDesc.type == BoundResourceType::WRITABLE_BUFFER) {
                layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.writableBufferCount++ });
            }
            else if (resourceDesc.type == BoundResourceType::SAMPLER) {
                layout.resourceMap.insert(resourceDesc.name, { resourceDesc.type, layout.samplerCount++ });
            }
            else if (resourceDesc.type == BoundResourceType::INVALID) {
                break;
            }
        }
        return layout;
    };

    D3D11_DEPTH_STENCIL_DESC toD3D11DepthStencilDesc(const DepthStencilDesc& depthDesc)
    {
        D3D11_DEPTH_STENCIL_DESC desc;
        desc.DepthEnable = depthDesc.depthCullMode != ComparisonFunc::OFF;
        desc.DepthFunc = getD3DComparisonFunc(depthDesc.depthCullMode);

        desc.DepthWriteMask = depthDesc.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = depthDesc.stencilEnabled;
        desc.StencilReadMask = depthDesc.stencilReadMask;
        desc.StencilWriteMask = depthDesc.stencilWriteMask;
        desc.FrontFace = getD3DStencilOpDesc(depthDesc.stencilFrontFace);
        desc.BackFace = getD3DStencilOpDesc(depthDesc.stencilBackFace);

        return desc;
    }

    uint32_t createPipelineState(Renderer& renderer, const std::string& pipelineName, const Array<ShaderMacro>& shaderMacros)
    {
        ID3D11Device1* device = renderer.getDevice();

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

        // Need to get or create Input Layout
        pipeline.inputLayout = renderer.inputLayoutManager.getOrCreateInputLayout(pipelineDefinition->inputLayoutDesc);

        // Depth Stencil State
        D3D11_DEPTH_STENCIL_DESC depthDesc = toD3D11DepthStencilDesc(pipelineDefinition->depthStencilState);
        device->CreateDepthStencilState(&depthDesc, &pipeline.depthStencilState);

        // Rasterizer State
        D3D11_RASTERIZER_DESC rasterizerDesc = toD3D11RasterizerDesc(pipelineDefinition->rasterStateDesc);
        device->CreateRasterizerState(&rasterizerDesc, &pipeline.rasterizerState);

        // Blend State
        // Resource Binding Layout
        // How do we handle our different resource binding layout descriptions?
        // Do we compile them all here? Was that the plan?
    }
}