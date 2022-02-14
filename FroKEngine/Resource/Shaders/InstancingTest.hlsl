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

// ���̴� 5.1 �̻���� �����ϴ� �ؽ�Ʈ �迭
// Texture2DArray�� �޸� �� �迭���� ũ��� ������ �ٸ� �ؽ�ó���� ���� �� �־� �����ϴ�.
Texture2D gDiffuseMap[7] : register(t0);

// ���� �ڷḦ space1�� �����Ѵ�. ���� ���� �ؽ�ó �迭���� ��ġ�� �ʴ´�.
// ���� �ؽ�ó �迭�� space0�� �������� t0, t1, ... ,t6�� �����Ѵ�.
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

    // �ִ� MaxLights���� ��ü�� ���� �߿���
    // [0, NUM_DIR_LIGHTS) ������ ���ε��� ���Ɽ���̰�,
    // [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHT) ������ ���ε��� �����̴�.
    // [NUM_DIR_LIGHTS + NUM_POINT_LIGHT, NUM_DIR_LIGHTS + NUM_POINT_LIGHT + NUM_SPOT_LIGHT)�� �������̴�.
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

    // ������ �ﰢ���� ���� �������� �ʵ���
    // nointerpolation�� �����Ѵ�.
    nointerpolation uint MatIndex : MAXINDEX;
};


VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;

    // �ν��Ͻ� �ڷḦ �����´�.
    InstanceData instData = gInstanceData[instanceID];
    float4x4 world = instData.World;
    float4x4 texTransform = instData.TexTransform;
    uint matIndex = instData.MaterialIndex;

    vout.MatIndex = matIndex;

    MaterialData matData = gMaterialData[matIndex];

    // ���� �������� ��ȯ�Ѵ�.
    float4 posW = mul(float4(vin.PosL, 1.0f), world);
    vout.PosW = posW.xyz;

    // ���� ��Ŀ� ��յ� ��ʰ� ���ٰ� �����ϰ� ������ ��ȯ�Ѵ�.
    // ��յ� ��ʰ� �ִٸ� ����ġ ����� ����ؾ� �Ѵ�.
    vout.NormalW = mul(vin.NormalL, (float3x3) world);

    // ���� ���� �������� ��ȯ�Ѵ�.
    vout.PosH = mul(posW, gViewProj);

    // ��� ���� Ư������ ���� �ﰢ���� ���� �����ȴ�.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), texTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    // ���� �ڷḦ �����´�.
    MaterialData matData = gMaterialData[pin.MatIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseTexIndex = matData.DiffuseMapIndex;

    // �ؽ�ó �迭�� �ؽ�ó�� �������� ��ȸ�Ѵ�.
    diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);

    // ������ �����ϸ� ���� ���̰� �ƴϰ� �� �� �ֱ� ������, 
    // �ٽ� ����ȭ�Ѵ�.
    pin.NormalW = normalize(pin.NormalW);

    // ����Ǵ� ������ �������� ����
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // ���� ��꿡 ���ԵǴ� �׵�
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // ���͸���
    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, roughness };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight;

    // ���� �ϴ� ��Ĵ��, �л� �������� ���ĸ� �����´�.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}

