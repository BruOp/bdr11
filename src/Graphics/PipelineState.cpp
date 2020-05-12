
#include "pch.h"

#include <utility>

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
        D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC{};
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

    D3D11_DEPTH_STENCIL_DESC toD3D11DepthStencilDesc(const DepthStencilDesc& depthDesc)
    {
        D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC{};
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


    D3D11_BLEND_DESC toD3D11BlendDesc(const BlendStateDesc& blendDesc)
    {
        D3D11_BLEND_DESC desc = CD3D11_BLEND_DESC{};
        desc.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnable;
        desc.IndependentBlendEnable = blendDesc.independentBlendEnable;

        for (size_t i = 0; i < _countof(blendDesc.renderTargetBlendDescs); i++) {
            const auto& rtBlendDesc = blendDesc.renderTargetBlendDescs[i];
            desc.RenderTarget[i] = D3D11_RENDER_TARGET_BLEND_DESC{
                rtBlendDesc.blendMode != BlendMode::OPAQUE,
                D3D11_BLEND(rtBlendDesc.srcBlend),
                D3D11_BLEND(rtBlendDesc.dstBlend),
                D3D11_BLEND_OP(rtBlendDesc.blendOp),
                D3D11_BLEND(rtBlendDesc.srcBlendAlpha),
                D3D11_BLEND(rtBlendDesc.dstBlendAlpha),
                D3D11_BLEND_OP(rtBlendDesc.blendOpAlpha),
                0x0F,
            };
        }
        return desc;
    }

    void compileShader(const char* code, const D3D_SHADER_MACRO* macros, ID3DBlob** blob, const PipelineStage stage)
    {
        ID3DBlob* error = nullptr;
        std::string entry = "";
        std::string shaderVersion = "";
        if (stage == PipelineStage::VERTEX_STAGE) {
            entry = "VSMain";
            shaderVersion = "vs_5_0";
        }
        else if (stage == PipelineStage::PIXEL_STAGE) {
            entry = "PSMain";
            shaderVersion = "ps_5_0";
        }
        else if (stage == PipelineStage::COMPUTE_STAGE) {
            entry = "CSMain";
            shaderVersion = "cs_5_0";
        }

        HRESULT result = D3DCompile(
            code,
            strlen(code),
            nullptr,
            macros,
            nullptr,
            entry.c_str(), shaderVersion.c_str(),
            0, 0,
            blob,
            &error
        );

        if (error) {
            HALT((char*)error->GetBufferPointer());
        }
    }

    std::string readFile(const char* filePath)
    {
        std::ifstream shaderFile{ filePath };
        std::string code;
        std::string line;
        if (shaderFile.is_open()) {
            while (std::getline(shaderFile, line)) {
                code += line;
                code += '\n';
            }
        }
        else {
            ERROR("Could not open shader file");
        }
        shaderFile.close();
        return code;
    }

    PipelineStateDefinitionHandle registerPipelineStateDefinition(
        Renderer& renderer,
        const std::string& filePath,
        PipelineStateDefinition&& pipelineDefinition
    )
    {
        pipelineDefinition.shaderCodeId = renderer.shaderCodeRegistry.size();
        renderer.shaderCodeRegistry.push_back(readFile(filePath.c_str()));

        PipelineStateDefinitionHandle handle{ renderer.pipelineDefinitions.size() };
        renderer.pipelineDefinitions.push_back(pipelineDefinition);
        return handle;
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

    ResourceBindingLayoutDesc getPerDrawLayoutDesc(
        const PipelineStateDefinition& pipelineDefinition,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    )
    {
        // Count the number of per draw required resource
        ResourceBindingLayoutDesc perDrawLayoutDesc = pipelineDefinition.perDrawRequiredResources;
        uint32_t numResources = 0;
        for (; numResources < perDrawLayoutDesc.maxResources; numResources++) {
            if (perDrawLayoutDesc.resourceDescs[numResources].type == BoundResourceType::INVALID) {
                break;
            }
        }

        for (size_t i = 0; i < numMacros; i++) {
            // Append the optional defintion to our layout;
            PipelineStateDefinition::BindingMapView view{};
            const bool retrieved = pipelineDefinition.optionalResourceMap.get_in(shaderMacros[i].name, &view);
            if (retrieved) {
                for (size_t i = 0; i < view.count; i++) {
                    perDrawLayoutDesc.resourceDescs[numResources++] = pipelineDefinition.optionalResources.resourceDescs[view.offset + i];
                }
            }
        }
        return  perDrawLayoutDesc;
    };

    PipelineHandle getOrCreatePipelineState(
        Renderer& renderer,
        const PipelineStateDefinitionHandle& pipelineDefinitionHandle,
        const ShaderMacro shaderMacros[],
        const size_t numMacros
    )
    {
        ID3D11Device1* device = renderer.getDevice();

        const PipelineStateDefinition& pipelineDefinition = renderer.pipelineDefinitions[pipelineDefinitionHandle.idx];

        // Check for whether the pipeline has already been created
        uint32_t pipelineKey = renderer.pipelines.getHashKey(shaderMacros, numMacros);
        PipelineHandle pipelineHandle = renderer.pipelines.getHandle(pipelineKey);
        if (isValid(pipelineHandle)) {
            // If present, return the index as a PipelineHandle so it can be used for fast indexing
            // in the future
            return pipelineHandle;
        }

        // Else, create a new pipeline:
        PipelineState pipeline{ };

        ResourceBindingLayoutDesc perDrawLayoutDesc = getPerDrawLayoutDesc(pipelineDefinition, shaderMacros, numMacros);

        // 1. Create list of macros to pass to shader compilation
        // 2. Append optional resources to resource layout desc based on macros
        D3D_SHADER_MACRO d3dMacros[16] = { nullptr };
        for (size_t i = 0; i < numMacros; i++) {
            d3dMacros[i] = D3D_SHADER_MACRO{ shaderMacros[i].name };
        }

        const std::string& shaderCode = renderer.shaderCodeRegistry[pipelineDefinition.shaderCodeId];

        if (pipelineDefinition.stages & PipelineStage::VERTEX_STAGE) {
            ID3DBlob* vsBlob;
            compileShader(shaderCode.c_str(), d3dMacros, &vsBlob, PipelineStage::VERTEX_STAGE);

            DX::ThrowIfFailed(device->CreateVertexShader(
                vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pipeline.vertexShader
            ));
        }
        if (pipelineDefinition.stages & PipelineStage::PIXEL_STAGE) {
            ID3DBlob* psBlob;
            compileShader(shaderCode.c_str(), d3dMacros, &psBlob, PipelineStage::PIXEL_STAGE);

            DX::ThrowIfFailed(device->CreatePixelShader(
                psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pipeline.pixelShader
            ));
        }
        if (pipelineDefinition.stages & PipelineStage::COMPUTE_STAGE) {
            ID3DBlob* csBlob;
            compileShader(shaderCode.c_str(), d3dMacros, &csBlob, PipelineStage::COMPUTE_STAGE);

            DX::ThrowIfFailed(device->CreateComputeShader(
                csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &pipeline.computeShader
            ));
        }

        // Resource Binding Layout
        // TODO: Views and Frames will own their resource binding layouts and resource binding objects,
        // but we'll use the layout descs defined here to validate that the constants will actually be available
        pipeline.perDrawBindingLayout = createLayout(renderer, perDrawLayoutDesc);

        // Need to get or create Input Layout
        pipeline.inputLayout = renderer.inputLayoutManager.getOrCreateInputLayout(pipelineDefinition.inputLayoutDesc);

        // Depth Stencil State
        D3D11_DEPTH_STENCIL_DESC depthDesc = toD3D11DepthStencilDesc(pipelineDefinition.depthStencilState);
        device->CreateDepthStencilState(&depthDesc, &pipeline.depthStencilState);

        // Rasterizer State
        D3D11_RASTERIZER_DESC rasterizerDesc = toD3D11RasterizerDesc(pipelineDefinition.rasterStateDesc);
        device->CreateRasterizerState(&rasterizerDesc, &pipeline.rasterizerState);

        // Blend State
        D3D11_BLEND_DESC blendDesc = toD3D11BlendDesc(pipelineDefinition.blendState);
        device->CreateBlendState(&blendDesc, &pipeline.blendState);

        return renderer.pipelines.insert(pipelineKey, pipeline);
    }

    ResourceBinder allocateResourceBinder(Renderer& renderer, const PipelineHandle pipelineId)
    {
        PipelineState& pipeline = renderer.pipelines[pipelineId];
        ResourceBindingHeap& heap = renderer.bindingHeap;
        ResourceBindingLayout& layout = pipeline.perDrawBindingLayout;
        ResourceBinder binder{  };
        binder.readableBufferOffset = heap.srvs.size();
        heap.srvs.resize(binder.readableBufferOffset + size_t(layout.readableBufferCount));

        binder.writableBufferOffset = heap.uavs.size();
        heap.uavs.resize(binder.writableBufferOffset + size_t(layout.writableBufferCount));

        binder.samplerOffset = heap.samplers.size();
        heap.samplers.resize(binder.samplerOffset + size_t(layout.samplerCount));
        return binder;
    }

    void reset(PipelineState& pipelineState)
    {
        release(pipelineState.vertexShader);
        release(pipelineState.pixelShader);
        release(pipelineState.computeShader);
        release(pipelineState.depthStencilState);
        release(pipelineState.rasterizerState);
        release(pipelineState.blendState);
    }
}