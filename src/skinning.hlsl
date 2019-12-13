struct Joint
{
	matrix transform;
};

StructuredBuffer<Joint> boneBuffer : register(t0);

Buffer<float3> in_POS : register(t1);
Buffer<float3> in_NORM : register(t2);
Buffer<float4> in_TANG : register(t3);
Buffer<uint4> in_INDICES : register(t4);
Buffer<float4> in_WEIGHTS : register(t5);

RWByteAddressBuffer out_Pos : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint numVertices = 0;
	in_POS.GetDimensions(numVertices);
	if (DTid.x < numVertices)
	{
		float3 position = in_POS.Load(DTid.x) + float3(0.0, 0.0, 1.0);
		uint4 indices = in_INDICES.Load(DTid.x);
		float4 weights = in_WEIGHTS.Load(DTid.x);
	
		const uint pos_offset = DTid.x * 12u;
		out_Pos.Store3(pos_offset, asuint(position));
	}
}