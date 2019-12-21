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
RWByteAddressBuffer out_Norm : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const uint pos_offset = DTid.x * 12u;
	uint numVertices;
	in_POS.GetDimensions(numVertices);
	if (DTid.x < numVertices)
	{
		float3 position = in_POS.Load(DTid.x);
		float3 normal = in_NORM.Load(DTid.x);
		
		uint4 joints = in_INDICES.Load(DTid.x);
		float4 weights = (in_WEIGHTS.Load(DTid.x));
		matrix skinMatrix = mul(weights.x, boneBuffer[joints.x].transform)
			+ mul(weights.y, boneBuffer[joints.y].transform)
			+ mul(weights.z, boneBuffer[joints.z].transform)
			+ mul(weights.w, boneBuffer[joints.w].transform);
		
		float3x3 normalMatrix = (float3x3) skinMatrix;
		float4 newPos = mul(float4(position, 1.0), skinMatrix);
		position = newPos.xyz / newPos.w;
		float3 newNormal = normalize(mul(normal, normalMatrix));
		out_Pos.Store3(pos_offset, asuint(position));
		out_Norm.Store3(pos_offset, asuint(newNormal));
	}
}