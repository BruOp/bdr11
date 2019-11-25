struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 NormalWS : NORMALWS;
};

float4 main(in PSInput input) : SV_Target0
{
    return float4(input.NormalWS, 1.0f);
}