struct Joint
{
	matrix transform;
};

StructuredBuffer<Joint> boneBuffer : register(t0);

Buffer<float3> in_POS : register(t1);
Buffer<float3> in_NORM : register(t2);
Buffer<uint4> in_INDICES : register(t3);
Buffer<float4> in_WEIGHTS : register(t4);
//Buffer<float4> in_TANG : register(t5);

RWByteAddressBuffer out_Pos : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const uint pos_offset = DTid.x * 12u;
	uint numVertices;
	in_POS.GetDimensions(numVertices);
	if (DTid.x < numVertices)
	{
		float3 position = in_POS.Load(DTid.x);
		uint4 joints = in_INDICES.Load(DTid.x);
		float4 weights = in_WEIGHTS.Load(DTid.x);
		matrix skinMatrix = mul(boneBuffer[joints.x].transform, weights.x)
			+ mul(boneBuffer[joints.y].transform, weights.y)
			+ mul(boneBuffer[joints.z].transform, weights.z)
			+ mul(boneBuffer[joints.w].transform, weights.w);
		position = mul(float4(position, 1.0), skinMatrix).xyz;
		//uint4 indices = in_INDICES.Load(DTid.x);
		//float4 weights = in_WEIGHTS.Load(DTid.x);
	
		out_Pos.Store3(pos_offset, asuint(position));
	}
}