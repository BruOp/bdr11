cbuffer VIEW_CONSTANT_BUFFER : register(b0)
{
    matrix view;
    matrix proj;
    matrix VP;
}

cbuffer DRAW_CONSTANTS_BUFFER : register(b1)
{
    matrix model;
    matrix invModel;
}

struct VSInput
{
    float3 Position : SV_Position;
    float3 Normal : NORMAL;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float3 NormalWS : NORMALWS;
};

VSOutput main(in VSInput input)
{
    VSOutput output;
    output.PositionCS = mul(mul(float4(input.Position, 1.0f), model), VP);

    output.NormalWS = mul(float4(input.Normal, 1.0f), invModel).xyz;

    return output;
}
