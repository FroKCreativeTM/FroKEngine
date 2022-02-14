#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtils.hlsl"

struct InstanceData
{
    float4x4 World;
    float4x4 TexTransform;
    uint MaterialIndex;
    uint InstPad0;
    uint InstPad1;
    uint InstPad2;
};

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint MatPad0;
    uint MatPad1;
    uint MatPad2;
};

// 셰이더 5.1 이상부터 지원하는 텍스트 배열
// Texture2DArray와 달리 이 배열에는 크기와 형식이 다른 텍스처들을 담을 수 있어 유연하다.
Texture2D gDiffuseMap[7] : register(t0);

// 재질 자료를 space1에 배정한다. 따라서 위의 텍스처 배열과는 겹치지 않는다.
// 위의 텍스처 배열은 space0의 레지스터 t0, t1, ... ,t6를 차지한다.
StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPass : register(b0)
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

    // 최대 MaxLights개의 물체별 광원 중에서
    // [0, NUM_DIR_LIGHTS) 구간의 색인들은 지향광들이고,
    // [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHT) 구간의 색인들은 점광이다.
    // [NUM_DIR_LIGHTS + NUM_POINT_LIGHT, NUM_DIR_LIGHTS + NUM_POINT_LIGHT + NUM_SPOT_LIGHT)는 점적광이다.
    Light gLights[MaxLights];
}

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

    // 색인이 삼각형을 따라 보간되지 않도록
    // nointerpolation을 지정한다.
    nointerpolation uint MatIndex : MAXINDEX;
};


VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;

    // 인스턴스 자료를 가져온다.
    InstanceData instData = gInstanceData[instanceID];
    float4x4 world = instData.World;
    float4x4 texTransform = instData.TexTransform;
    uint matIndex = instData.MaterialIndex;

    vout.MatIndex = matIndex;

    MaterialData matData = gMaterialData[matIndex];

    // 세계 공간으로 변환한다.
    float4 posW = mul(float4(vin.PosL, 1.0f), world);
    vout.PosW = posW.xyz;

    // 세계 행렬에 비균등 비례가 없다고 가정하고 법선을 변환한다.
    // 비균등 비례가 있다면 역전치 행렬을 사용해야 한다.
    vout.NormalW = mul(vin.NormalL, (float3x3) world);

    // 동차 절단 공간으로 변환한다.
    vout.PosH = mul(posW, gViewProj);

    // 출력 정점 특성들은 이후 삼각형에 따라 보간된다.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), texTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    // 재질 자료를 가져온다.
    MaterialData matData = gMaterialData[pin.MatIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseTexIndex = matData.DiffuseMapIndex;

    // 텍스처 배열의 텍스처를 동적으로 조회한다.
    diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);

    // 법선을 보간하면 단위 길이가 아니게 될 수 있기 때문에, 
    // 다시 정규화한다.
    pin.NormalW = normalize(pin.NormalW);

    // 조명되는 점에서 눈으로의 벡터
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // 조명 계산에 포함되는 항들
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // 머터리얼
    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, roughness };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight;

    // 흔히 하는 방식대로, 분산 재질에서 알파를 가져온다.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}

