#ifndef __COMPUTE_FX__
#define __COMPUTE_FX__

#include "params.fx"

struct VS_IN
{
    float3 Pos : POSITION;
};

struct VS_OUT
{
    float4 pos : SV_Position; // System Value(�ý��� �󿡼� float4�� ���༭ ����)
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT ret = (VS_OUT) 0.f;

    ret.pos = float4(input.Pos, 1.0f);

    return ret;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    return input.pos;

}


#endif