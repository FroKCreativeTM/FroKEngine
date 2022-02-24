#ifndef __COMPUTE_FX__
#define __COMPUTE_FX__

#include "params.fx"

// u : unordered access view
// 보통과 같은 그래픽 연산에서 사용하는 물건이 아니다.
// RW : Read-Write(수정과 읽기가 전부 가능)
// 보통은 Read-Only
RWTexture2D<float4> g_rwtex_0 : register(u0);

// 스레드 그룹 당 스레드 갯수
// max : 1024 (CS_5.0)
// 하나의 스레드 그룹은 하나의 다중처리기에서 실행
// numthreads(2, 2, 2) : xyz 좌표가 각각 2개씩인 스레드 그리드 생성
[numthreads(1024, 1, 1)]
void CS_Main(int3 threadIndex : SV_DispatchThreadID)
{
    if (threadIndex.y % 2 == 0)
        g_rwtex_0[threadIndex.xy] = float4(1.f, 0.f, 0.f, 1.f);
    else 
        g_rwtex_0[threadIndex.xy] = float4(0.f, 1.f, 0.f, 1.f);
}

#endif