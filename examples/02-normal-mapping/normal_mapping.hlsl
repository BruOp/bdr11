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
// Resources
//=================================================================================================

Texture2D albedo : register(t0);
Texture2D normalMap : register(t1);

SamplerState albedoSampler : register(s0);
SamplerState normalSampler : register(s1);

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
    float3 PositionWS : POSITIONWS;
    float3 NormalWS : NORMAL;
    float2 vUv : TEXCOORD;
};

struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 PositionWS : POSITIONWS;
    float3 NormalWS : NORMAL;
    float2 vUv : TEXCOORD;
};

//=================================================================================================
// Helper Functions
//=================================================================================================
// Taken from http://www.thetenthplanet.de/archives/1180
// And https://github.com/microsoft/DirectXTK12/blob/master/Src/Shaders/Utilities.fxh#L20
float3x3 cotangentFrame(float3 N, float3 p, float2 uv)
{
    // Get edge vectors of the pixel triangle
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    // Solve the linear system
    float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
    float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
    float3 t = normalize(mul(float2(duv1.x, duv2.x), inverseM));
    float3 b = normalize(mul(float2(duv1.y, duv2.y), inverseM));
    
    // Construct a scale-invari­ant frame 
    return float3x3(t, b, N);
}

float3 perturb_normal(float3 N, float3 V, float2 texcoord)
{ // assume N, the interpolated vertex normal and
    // V, the view vector (vertex to eye)
    float3 map = normalMap.Sample(normalSampler, texcoord).xyz * 2.0 - 1.0;
    map.z = sqrt(1. - dot(map.xy, map.xy));
    float3x3 TBN = cotangentFrame(N, -V, texcoord);
    return normalize(mul(map, TBN));
}

float clampDot(float3 A, float3 B)
{
    return clamp(dot(A, B), 0.0, 1.0f);
}

//=================================================================================================
// Vertex Shader
//=================================================================================================

VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.vUv = input.UV;
    output.PositionWS = mul(model, float4(input.Position, 1.0f)).xyz;
    output.PositionCS = mul(VP, float4(output.PositionWS, 1.0f));
    output.NormalWS = normalize(mul(invModel, float4(input.Normal, 1.0f)).xyz);
    return output;
}

//=================================================================================================
// Pixel Shader
//=================================================================================================

float4 PSMain(in PSInput input) : SV_Target0
{
    float3 viewDir = cameraPos - input.PositionWS;

#ifdef NORMAL_MAPPING
		float3 normal = perturb_normal(input.NormalWS, viewDir, input.vUv);
#else
    float3 normal = input.NormalWS;
#endif
    float4 baseColor = albedo.Sample(albedoSampler, input.vUv);

    const float3 lightPos = float3(5.0 * sin(time), 6.0, 5.0 * cos(time));
    const float lightIntensity = 30.0f;
    const float3 lightColor = float3(1.0, 1.0, 1.0);

    float3 lightDir = lightPos - input.PositionWS;
    float lightDist = length(lightDir);
    lightDir = lightDir / lightDist;

    float attenuation = lightIntensity / (lightDist * lightDist);
    float3 light = attenuation * lightColor * clampDot(normal, lightDir);
    
    return float4(baseColor.rgb * light, 1.0f);
}