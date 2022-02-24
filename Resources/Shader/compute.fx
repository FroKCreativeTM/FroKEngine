#ifndef __COMPUTE_FX__
#define __COMPUTE_FX__

#include "params.fx"

// u : unordered access view
// ����� ���� �׷��� ���꿡�� ����ϴ� ������ �ƴϴ�.
// RW : Read-Write(������ �бⰡ ���� ����)
// ������ Read-Only
RWTexture2D<float4> g_rwtex_0 : register(u0);

// ������ �׷� �� ������ ����
// max : 1024 (CS_5.0)
// �ϳ��� ������ �׷��� �ϳ��� ����ó���⿡�� ����
// numthreads(2, 2, 2) : xyz ��ǥ�� ���� 2������ ������ �׸��� ����
[numthreads(1024, 1, 1)]
void CS_Main(int3 threadIndex : SV_DispatchThreadID)
{
    if (threadIndex.y % 2 == 0)
        g_rwtex_0[threadIndex.xy] = float4(1.f, 0.f, 0.f, 1.f);
    else 
        g_rwtex_0[threadIndex.xy] = float4(0.f, 1.f, 0.f, 1.f);
}

#endif