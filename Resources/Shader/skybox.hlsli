#ifndef __SKYBOX_HLSLI__
#define __SKYBOX_HLSLI__

#include "params.hlsli"

struct VS_IN
{
    float3 localPos          : POSITION;
    float2 uv           : TEXCOORD;
};

struct VS_OUT
{
    float4 pos              : SV_Position;  // System Value(시스템 상에서 float4로 해줘서 강제)
    float2 uv               : TEXCOORD;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;

    // Translation은 적용하지 않고 Rotation만 적용한다.
    // local에서 world를 스킵하고 바로 view 좌표로 이동
    float4 viewPos = mul(float4(input.localPos, 0.0f), g_matView);
    float4 clipSpacePos = mul(viewPos, g_matProjection);

    // w/w=1이기 때문에 항상 깊이기 1로 유지된다. (중간의 0~1사이가 아니라 무조건 1이다.)
    output.pos = clipSpacePos.xyww;
    output.uv = input.uv;

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    float4 color = g_tex_0.Sample(g_sam_0, input.uv);
    return color;
}

#endif