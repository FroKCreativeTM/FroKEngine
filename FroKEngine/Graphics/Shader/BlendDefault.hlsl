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


// ���� ���õ� ����ü�� �޼��餩 �����´�.
#include "LightingUtils.hlsl"

Texture2D gDiffuseMap : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// �����Ӵ� ���
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

// �� ���͸���� ��� ������
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

    // ���� ���α׷��� �Ȱ� �Ű��������� �����Ӻ��� ������ �� �ְ� �Ѵ�.
    // ���� ��� �Ϸ� �� Ư�� �ð��뿡�� �Ȱ��� ������ �� �ִ�.
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerPassPad2;

    // �ε��� [0, NUM_DIR_LIGHTS)�� ���� �����̴�.
    // �ε��� [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS)�� ����.
    // �ε��� [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS) 
    // ��ü�� �ִ� MaxLights�� ���� ������.
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
	
    // ���� �������� ��ȯ�Ѵ�.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // �������� ���� �����Ϸ� �����Ѵ�. ���� ���� ����� ����ġ�� �ʿ��ϴ�.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // ������(homogeneous) Ŭ�� �������� ��ȯ�մϴ�.
    vout.PosH = mul(posW, gViewProj);
	
    // �ﰢ���� ���������� ������ ���� ��� ���� �Ӽ�.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	
#ifdef ALPHA_TEST
    // �ؽ�ó�� ���İ� 0.1���� ������ �ȼ��� ����Ѵ�.
    // ���̴� �ȿ��� �� ������ �ִ��� ���� �����ϴ� ���� �ٶ����ϴ�.
    // �׷��� �� �� ���̴��� ������ �ڵ��� ������ ������ �� �����Ƿ� ȿ�����̴�.
	clip(diffuseAlbedo.a - 0.1f);
#endif


    // ������ �����ϸ� ����ȭ���� ���� �� �����Ƿ� �ٽ� ����ȭ�մϴ�.
    pin.NormalW = normalize(pin.NormalW);

    // ������ ������ ����Ǵ� �����Դϴ�.
    float3 toEyeW = normalize(gEyePosW - pin.PosW);
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;    // ����ȭ ����

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // �����̵� ���� �ڵ�
    // litColor = ceil(litColor * 5) / 5.0f;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    // Ȯ�� �˺������� ���ĸ� ���ϴ� �Ϲ����� ��Ģ�Դϴ�.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


