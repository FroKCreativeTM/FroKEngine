#ifndef __DEFAULT_FX__
#define __DEFAULT_FX__

#include "params.fx"
#include "utils.fx"

struct VS_IN
{
    float3 pos          : POSITION;
    float2 uv           : TEXCOORD;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
};

struct VS_OUT
{
    float4 pos              : SV_Position;  // System Value(시스템 상에서 float4로 해줘서 강제)
    float2 uv               : TEXCOORD;
    float3 viewPos          : POSITION;     // 뷰 좌표계 상 위치
    float3 viewNormal       : NORMAL;       // 뷰 좌표계 상 노멀
    float3 viewTangent      : TANGENT;
    float3 viewBinormal     : BINORMAL;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;

    output.pos = mul(float4(input.pos, 1.f), g_matWVP);
    output.uv = input.uv;

    // 바로 투영 좌표계로 가는 것이 아니다!
    // 1.f는 위치
    output.viewPos = mul(float4(input.pos, 1.f), g_matWV).xyz;
    // 0.f는 translation이 적용되지 않게!
    output.viewNormal = normalize(mul(float4(input.normal, 0.f), g_matWV).xyz);
    output.viewTangent = normalize(mul(float4(input.tangent, 0.f), g_matWV).xyz);
    output.viewBinormal = normalize(cross(output.viewTangent, output.viewNormal));

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    float4 color = float4(1.f, 1.f, 1.f, 1.f);

    // 텍스처가 있다면
    if (g_tex_on_0)
        color = g_tex_0.Sample(g_sam_0, input.uv);
    
    float3 viewNormal = input.viewNormal;

    if (g_tex_on_1)
    {
        // [0,255] 범위에서 [0,1]로 변환
        float3 tangentSpaceNormal = g_tex_1.Sample(g_sam_0, input.uv).xyz;
        // [0,1]를 [-1, 1]로 변환
        tangentSpaceNormal = (tangentSpaceNormal - 0.5f) * 2.f;
        float3x3 matTBN = { input.viewTangent, input.viewBinormal, input.viewNormal };
        viewNormal = normalize(mul(tangentSpaceNormal, matTBN));
    }

    LightColor totalColor = (LightColor) 0.f;

    for (int i = 0; i < g_lightCount; ++i)
    {
        // i번째 빛의 노말과 위치를 받는다.
        LightColor color = CalculateLightColor(i, viewNormal, input.viewPos);
        totalColor.diffuse += color.diffuse;
        totalColor.ambient += color.ambient;
        totalColor.specular += color.specular;
    }

    color.xyz = (totalColor.diffuse.xyz * color.xyz) +
        totalColor.ambient.xyz * color.xyz +
        totalColor.specular.xyz;
        
    return color;
}


// 텍스처를 위한 셰이더
// [Texture Shader]
// g_tex_0 : Output Texture
// AlphaBlend : true
struct VS_TEX_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TEX_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VS_TEX_OUT VS_Tex(VS_TEX_IN input)
{
    VS_TEX_OUT output = (VS_TEX_OUT) 0;

    output.pos = mul(float4(input.pos, 1.f), g_matWVP);
    output.uv = input.uv;

    return output;
}

float4 PS_Tex(VS_TEX_OUT input) : SV_Target
{
    float4 color = float4(1.f, 1.f, 1.f, 1.f);
    if (g_tex_on_0)
        color = g_tex_0.Sample(g_sam_0, input.uv);

    return color;
}

#endif