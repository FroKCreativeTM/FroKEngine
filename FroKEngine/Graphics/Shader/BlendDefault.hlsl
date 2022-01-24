//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif


// 조명에 관련된 구조체와 메서들ㄹ 가져온다.
#include "LightingUtils.hlsl"

Texture2D gDiffuseMap : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// 프레임당 상수
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

// 각 머터리얼당 상수 데이터
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    // 응용 프로그램이 안개 매개변수들을 프래임별로 변경할 수 있게 한다.
    // 예를 들어 하루 중 특정 시간대에만 안개를 적용할 수 있다.
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerPassPad2;

    // 인덱스 [0, NUM_DIR_LIGHTS)는 방향 조명이다.
    // 인덱스 [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS)는 점광.
    // 인덱스 [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS) 
    // 개체당 최대 MaxLights에 대한 점적광.
    Light gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    float4x4 gMatTransform;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
	
    // 세계 공간으로 변환한다.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // 균일하지 않은 스케일로 가정한다. 따라서 세계 행렬의 역전치가 필요하다.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // 균질한(homogeneous) 클립 공간으로 변환합니다.
    vout.PosH = mul(posW, gViewProj);
	
    // 삼각형을 가로지르는 보간을 위한 출력 정점 속성.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	
#ifdef ALPHA_TEST
    // 텍스처의 알파가 0.1보다 작으면 픽셀을 폐기한다.
    // 셰이더 안에서 이 판정을 최대한 일찍 수행하는 것이 바람직하다.
    // 그러면 폐끼 시 셰이더의 나머지 코드의 시행을 생략할 수 있으므로 효율적이다.
	clip(diffuseAlbedo.a - 0.1f);
#endif


    // 법선을 보간하면 정규화되지 않을 수 있으므로 다시 정규화합니다.
    pin.NormalW = normalize(pin.NormalW);

    // 점에서 눈까지 조명되는 벡터입니다.
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;    // 정규화 진행

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // 툰셰이딩 구현 코드
    // litColor = ceil(litColor * 5) / 5.0f;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    // 확산 알베도에서 알파를 취하는 일반적인 규칙입니다.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


