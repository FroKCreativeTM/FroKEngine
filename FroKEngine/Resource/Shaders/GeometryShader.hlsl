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

// 텍스처가 많기 때문에 배열로 받는다.
Texture2D gTreeMapArray : register(t0);

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
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

	// 들어온 자료를 그대로 기하 셰이더에 넘겨준다.
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;

    return vout;
}

// 기하 셰이더는 반드시 실행에서 출력할 정점들의 최대 갯수를 표기해야한다.
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,    // 기본 도형마다의 ID
        inout TriangleStream<GeoOut> triStream)
{
    //
	// 빌보드가 y축과 정렬되고 눈을 향하도록 월드 공간에
	// 상대적인 스프라이트의 로컬 좌표계를 계산합니다.
	//

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f; // y축 정렬이니, xz-평면에 투영한다. 
    look = normalize(look);
    float3 right = cross(up, look);

	//
	// 세계 좌표 속 삼각형 띠 정점들을 계산한다. 
	//
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
	
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);

	//
	// 사각형 정점을 세계 공간으로 변환하고 삼각형 띠로 출력합니다.
	//
	
    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
	
    GeoOut gout;
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.TexC = texC[i];
        gout.PrimID = primID;
		
        triStream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    float3 uvw = float3(pin.TexC, pin.PrimID % 3);
    float4 diffuseAlbedo = gTreeMapArray.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	
#ifdef ALPHA_TEST
    // 텍스처의 알파가 0.1보다 작으면 픽셀을 폐기한다.
    // 셰이더 안에서 이 판정을 최대한 일찍 수행하는 것이 바람직하다.
    // 그러면 폐끼 시 셰이더의 나머지 코드의 시행을 생략할 수 있으므로 효율적이다.
	clip(diffuseAlbedo.a - 0.1f);
#endif


    // 법선을 보간하면 정규화되지 않을 수 있으므로 다시 정규화합니다.
    pin.NormalW = normalize(pin.NormalW);

    // 점에서 눈까지 조명되는 벡터입니다.
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;    // 정규화 진행

    // 조명 계산에 포함되는 항들.
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


