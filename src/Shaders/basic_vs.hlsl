cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix MVP;
	matrix invMVP;
	float4 extra1;
	float4 extra2;
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
    output.PositionCS = mul(float4(input.Position, 1.0f), MVP);

    output.NormalWS = mul(float4(input.Normal, 1.0f), MVP).xyz;

    return output;
}
