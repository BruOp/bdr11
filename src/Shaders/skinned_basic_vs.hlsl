cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix modelViewProj;
    uint numJoinMatrices;
    float3 padding;
    matrix jointMatrices[64];
}

struct VSInput
{
    float3 Position : SV_Position;
    float3 Normal : NORMAL;
    float4 Weights : BLENDWEIGHT;
    uint4 Indices : BLENDINDICES;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float3 NormalWS : NORMALWS;
};

VSOutput main(in VSInput input)
{
    VSOutput output;
    matrix skinMatrix =
		mul(input.Weights.x, jointMatrices[input.Indices.x]) +
		mul(input.Weights.y, jointMatrices[input.Indices.y]) +
		mul(input.Weights.z, jointMatrices[input.Indices.z]) +
		mul(input.Weights.w, jointMatrices[input.Indices.w]);
    
	output.PositionCS = mul(
		mul(float4(input.Position, 1.0f), skinMatrix),
		modelViewProj);

    output.NormalWS = mul(float4(input.Normal, 1.0f), modelViewProj).xyz;

    return output;
}
