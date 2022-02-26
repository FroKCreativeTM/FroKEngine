#ifndef __SKYBOX_FX__
#define __SKYBOX_FX__

#include "params.fx"

struct VS_IN
{
    float3 localPos          : POSITION;
    float2 uv           : TEXCOORD;
};

struct VS_OUT
{
    float4 pos              : SV_Position;  // System Value(�ý��� �󿡼� float4�� ���༭ ����)
    float2 uv               : TEXCOORD;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;

    // Translation�� �������� �ʰ� Rotation�� �����Ѵ�.
    // local���� world�� ��ŵ�ϰ� �ٷ� view ��ǥ�� �̵�
    float4 viewPos = mul(float4(input.localPos, 0.0f), g_matView);
    float4 clipSpacePos = mul(viewPos, g_matProjection);

    // w/w=1�̱� ������ �׻� ���̱� 1�� �����ȴ�. (�߰��� 0~1���̰� �ƴ϶� ������ 1�̴�.)
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