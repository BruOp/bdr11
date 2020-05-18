//=================================================================================================
// Constant buffers
//=================================================================================================

cbuffer VIEW_CONSTANT_BUFFER : register(b0)
{
    matrix view;
    matrix proj;
    matrix VP;
    float3 cameraPos;
    float time;
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
    unorm float4 Color : COLOR;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    unorm float4 Color : COLOR;
};

struct PSInput
{
    float4 PositionCS : SV_Position;
    unorm float4 Color : COLOR;
};

//=================================================================================================
// Vertex Shader
//=================================================================================================

VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.PositionCS = mul(VP, mul(model, float4(input.Position, 1.0f)));

    output.Color = input.Color;
    return output;
}

//=================================================================================================
// Pixel Shader
//=================================================================================================

float4 PSMain(in PSInput input) : SV_Target0
{
    return input.Color;
}
