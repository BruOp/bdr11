#define DIELECTRIC_SPECULAR 0.04
#define MIN_ROUGHNESS 0.045

#ifndef PI
#define PI 3.141592653589793
#endif // PI

float3 float3Splat(float x)
{
    return float3(x, x, x);
}


//=================================================================================================
// Constant buffers
//=================================================================================================

cbuffer VIEW_CONSTANT_BUFFER : register(b0)
{
    matrix view;
    matrix proj;
    matrix VP;
    float3 cameraPos;
    float padding;
}

cbuffer DRAW_CONSTANTS_BUFFER : register(b1)
{
    matrix model;
    matrix invModel;
}

cbuffer MATERIAL_DATA : register(b1)
{
    float4 baseColorFactor;
    float3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    uint alphaMode;
    float materialPadding[53];
}

//=================================================================================================
// Resources
//=================================================================================================
Texture2D albedo : register(t0);
Texture2D metallicRoughness : register(t1);
Texture2D normalMap : register(t2);
Texture2D emissiveMap : register(t3);

SamplerState albedoSampler : register(s0);
SamplerState pbrSampler : register(s1);
SamplerState normalSampler : register(s2);
SamplerState emissiveSampler : register(s3);

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
    float3 NormalWS : NORMALWS;
    float2 vUV : TEXCOORD;
};

struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 PositionWS : POSITIONWS;
    float3 NormalWS : NORMALWS;
    float2 vUV : TEXCOORD;
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

float D_GGX(float NoH, float roughness)
{
    float alpha = roughness * roughness;
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float3 F_Schlick(float VoH, float3 f0)
{
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

// From the filament docs. Geometric Shadowing function
// https://google.github.io/filament/Filament.html#toc4.4.2
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

float3 calcDiffuse(float3 baseColor, float metallic)
{
    return baseColor * (1.0 - DIELECTRIC_SPECULAR) * (1.0 - metallic);
}

float3 calcSpecular(float3 lightDir, float3 viewDir, float3 f0, float3 normal, float roughness)
{
    float3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform

    float D = D_GGX(NoH, roughness);
    float3 F = F_Schlick(VoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return D * V * F;
}


//=================================================================================================
// Vertex Shader
//=================================================================================================
VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.PositionWS = mul(float4(input.Position, 1.0f), model).xyz;
    output.PositionCS = mul(float4(output.PositionWS, 1.0f), VP);
    output.vUV = input.UV;
    
    output.NormalWS = normalize(mul(float4(input.Normal, 1.0f), invModel).xyz);

    return output;
}

//=================================================================================================
// Pixel Shader
//=================================================================================================
float4 PSMain(in PSInput input) : SV_Target0
{
    float3 viewDir = normalize(cameraPos - input.PositionWS);

#ifdef NORMAL_MAPPING
		float3 normal = perturb_normal(input.NormalWS, viewDir, input.vUV);
#else
    float3 normal = input.NormalWS;
#endif

#ifdef ALBEDO_MAP
	float4 baseColor = baseColorFactor * albedo.Sample(albedoSampler, input.vUV);
#else
    float4 baseColor = baseColorFactor;
#endif
	
    float occlusion = 1.0f;
    float2 roughnessMetal = float2(roughnessFactor, metallicFactor);
#ifdef METALLIC_ROUGHNESS_MAP
	float3 occlusionRoughnessMetal = metallicRoughness.Sample(pbrSampler, input.vUV).rgb;
    roughnessMetal *= occlusionRoughnessMetal.gb;
#ifdef OCCLUSION_MAP
    occlusion = occlusionRoughnessMetal.r;
#endif
#endif    
	
    float roughness = max(roughnessMetal.x, MIN_ROUGHNESS);
    float metallic = roughnessMetal.y;

    const float3 lightPos = float3(5.0, 10.0, 0.0);
    const float lightIntensity = 100.0f;
    const float3 lightColor = float3(1.0, 1.0, 1.0);

    float3 lightDir = lightPos - input.PositionWS;
    float lightDist = length(lightDir);
    lightDir = lightDir / lightDist;

    float attenuation = lightIntensity / (lightDist * lightDist);
    float3 light = attenuation * lightColor * clampDot(normal, lightDir);

    float3 diffuseColor = calcDiffuse(baseColor.rgb, metallic);
    float3 f0 = lerp(float3Splat(DIELECTRIC_SPECULAR), baseColor.rgb, metallic);
    float3 specularColor = calcSpecular(lightDir, viewDir, f0, normal, roughness);
    float3 color = occlusion * (diffuseColor + PI * specularColor) * light;

#ifdef EMISSIVE_MAP
    color += emissiveFactor * emissiveMap.Sample(emissiveSampler, input.vUV).rgb;
#endif
    
    return float4(color, 1.0f);
}

