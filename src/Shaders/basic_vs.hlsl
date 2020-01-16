cbuffer VIEW_CONSTANT_BUFFER : register(b0)
{
    matrix view;
    matrix proj;
    matrix VP;
    float4 cameraPos;
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
    float2 UV : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float3 PositionWS : POSITIONWS;
    float3 NormalWS : NORMALWS;
    float3 TangentWS : TANGENTWS;
    float3 BitangentWS : BITANGENTWS;
    float2 vUV : TEXCOORD;
};

VSOutput main(in VSInput input)
{
    VSOutput output;
    output.PositionWS = mul(float4(input.Position, 1.0f), model).xyz;
    output.PositionCS = mul(float4(output.PositionWS, 1.0f), VP);
    output.vUV = input.UV;
    
    output.NormalWS = normalize(mul(float4(input.Normal, 1.0f), invModel).xyz);
    output.TangentWS = normalize(mul(float4(input.Tangent.xyz, 0.0), model).xyz);
    output.BitangentWS = normalize(cross(output.NormalWS, output.TangentWS)) * input.Tangent.w;

    return output;
}
