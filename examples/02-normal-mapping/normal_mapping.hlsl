//=================================================================================================
// Constant buffers
//=================================================================================================

cbuffer VIEW_CONSTANT_BUFFER : register(b0)
{
    matrix view;
    matrix proj;
    matrix VP;
    float3 cameraPos;
    float padding;
}

cbuffer DRAW_CONSTANTS_BUFFER : register(b1)
{
    matrix model;
    matrix invModel;
}

//=================================================================================================
// Input/Output structs
//=================================================================================================

struct VSInput
{
    float3 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float3 NormalWS : NORMAL;
    float2 vUv : TEXCOORD;
};

struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 NormalWS : NORMAL;
    float2 vUv : TEXCOORD;
};

//=================================================================================================
// Vertex Shader
//=================================================================================================

VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.vUv = input.UV;
    output.PositionCS = mul(VP, mul(model, float4(input.Position, 1.0f)));

    output.NormalWS = normalize(mul(invModel, float4(input.Normal, 1.0f)).xyz);
    return output;
}

//=================================================================================================
// Pixel Shader
//=================================================================================================

float4 PSMain(in PSInput input) : SV_Target0
{
    return float4(input.vUv, 0.0f, 1.0f);
}