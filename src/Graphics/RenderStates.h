#pragma once
#include "pch.h"


namespace bdr
{
    enum class ComparisonFunc : uint8_t
    {
        OFF = 0,
        NEVER,
        LESS,
        LESS_OR_EQUAL,
        EQUAL,
        GREATER_OR_EQUAL,
        GREATER,
        ALWAYS,
    };

    enum class StencilOp : uint8_t
    {
        KEEP = 0,
        ZERO,
        REPLACE,
        INCR_SAT,
        DECR_SAT,
        INVERT,
        INCR,
        DECR
    };

    enum struct FillMode : uint8_t
    {
        WIREFRAME,
        SOLID
    };

    enum struct CullMode : uint8_t
    {
        NONE,
        FRONT_CW,
        FRONT_CCW,
        BACK_CW,
        BACK_CCW
    };

    enum RasterizerFlags : uint8_t
    {
        DEPTH_CLIP_ENABLED = 1 << 0,
        SCISSOR_ENABLED = 1 << 1,
        MULTISAMPLE = 1 << 2,
        ANTIALIASED_LINES = 1 << 3,
    };

    enum struct BlendMode : uint8_t
    {
        OPAQUE,
        MASKED,
        BLENDED
    };

    enum struct Blend : uint8_t
    {
        ZERO = uint8_t(D3D11_BLEND_ZERO),
        ONE = uint8_t(D3D11_BLEND_ONE),
        SRC_COLOR = uint8_t(D3D11_BLEND_SRC_COLOR),
        INV_SRC_COLOR = uint8_t(D3D11_BLEND_INV_SRC_COLOR),
        SRC_ALPHA = uint8_t(D3D11_BLEND_SRC_ALPHA),
        INV_SRC_ALPHA = uint8_t(D3D11_BLEND_INV_SRC_ALPHA),
        DEST_ALPHA = uint8_t(D3D11_BLEND_DEST_ALPHA),
        INV_DEST_ALPHA = uint8_t(D3D11_BLEND_INV_DEST_ALPHA),
        DEST_COLOR = uint8_t(D3D11_BLEND_DEST_COLOR),
        INV_DEST_COLOR = uint8_t(D3D11_BLEND_INV_DEST_COLOR),
        SRC_ALPHA_SAT = uint8_t(D3D11_BLEND_SRC_ALPHA_SAT),
        BLEND_FACTOR = uint8_t(D3D11_BLEND_BLEND_FACTOR),
        INV_BLEND_FACTOR = uint8_t(D3D11_BLEND_INV_BLEND_FACTOR),
        SRC1_COLOR = uint8_t(D3D11_BLEND_SRC1_COLOR),
        INV_SRC1_COLOR = uint8_t(D3D11_BLEND_INV_SRC1_COLOR),
        SRC1_ALPHA = uint8_t(D3D11_BLEND_SRC1_ALPHA),
        INV_SRC1_ALPHA = uint8_t(D3D11_BLEND_INV_SRC1_ALPHA)
    };

    enum struct BlendOp : uint8_t
    {
        // Add source 1 and source 2.
        ADD = D3D11_BLEND_OP_ADD,
        // Subtract source 1 from source 2.
        SUBTRACT = D3D11_BLEND_OP_SUBTRACT,
        // Subtract source 2 from source 1.
        REV_SUBTRACT = D3D11_BLEND_OP_REV_SUBTRACT,
        // Find the minimum of source 1 and source 2.
        MIN = D3D11_BLEND_OP_MIN,
        // Find the maximum of source 1 and source 2.
        MAX = D3D11_BLEND_OP_MAX
    };

    struct StencilFuncDesc
    {
        // The stencil operation to perform when stencil testing fails.
        StencilOp onStencilFail = StencilOp::KEEP;
        // The stencil operation to perform when stencil testing passes and depth testing fails.
        StencilOp onStencilPassDepthFail = StencilOp::KEEP;
        // The stencil operation to perform when stencil testing and depth testing both pass.
        StencilOp onStencilPassDepthPass = StencilOp::KEEP;
        // How to compare the existing stencil value vs the value output by the func
        ComparisonFunc comparisonFunc = ComparisonFunc::NEVER;
    };

    struct DepthStencilDesc
    {
        ComparisonFunc depthCullMode = ComparisonFunc::OFF;
        bool depthWrite = false;
        bool stencilEnabled = false;
        uint8_t stencilWriteMask = 0xFF;
        uint8_t stencilReadMask = 0xFF;
        StencilFuncDesc stencilBackFace;
        StencilFuncDesc stencilFrontFace;
    };

    struct RasterStateDesc
    {
        FillMode fillMode = FillMode::SOLID;
        CullMode cullMode = CullMode::BACK_CCW;
        int32_t depthBias = 0;
        float depthBiasClamp = 0.0f;
        float slopeScaledDepthBias = 0.0f;
        uint8_t flags = RasterizerFlags::DEPTH_CLIP_ENABLED;
    };

    struct RenderTargetBlendDesc
    {
        BlendMode blendMode = BlendMode::OPAQUE;
        // RGB value from pixel shader output
        Blend    srcBlend = Blend::ONE;
        // RGB Value stored in render target
        Blend    dstBlend = Blend::ZERO;
        // Describes how the two RGB values are combined
        BlendOp blendOp = BlendOp::ADD;
        // Alpha value from pixel shader output
        Blend    srcBlendAlpha = Blend::ONE;
        // Alpha value stored in render target
        Blend    dstBlendAlpha = Blend::ZERO;
        // Describes how the two alpha values are combined
        BlendOp blendOpAlpha = BlendOp::ADD;
        // A write mask -- can be used to mask the values after the
        // blending operation before they are stored in the render target
        // NOT IMPLEMENTED
        //uint8_t renderTargetWriteMask = 0x0F;
    };

    struct BlendStateDesc
    {
        bool alphaToCoverageEnable = false;
        bool independentBlendEnable = false;
        RenderTargetBlendDesc renderTargetBlendDescs[8] = { };
    };
}