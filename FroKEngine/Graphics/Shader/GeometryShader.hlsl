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

// �ؽ�ó�� ���� ������ �迭�� �޴´�.
Texture2D gTreeMapArray : register(t0);

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

	// ���� �ڷḦ �״�� ���� ���̴��� �Ѱ��ش�.
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;

    return vout;
}

// ���� ���̴��� �ݵ�� ���࿡�� ����� �������� �ִ� ������ ǥ���ؾ��Ѵ�.
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,    // �⺻ ���������� ID
        inout TriangleStream<GeoOut> triStream)
{
    //
	// �����尡 y��� ���ĵǰ� ���� ���ϵ��� ���� ������
	// ������� ��������Ʈ�� ���� ��ǥ�踦 ����մϴ�.
	//

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f; // y�� �����̴�, xz-��鿡 �����Ѵ�. 
    look = normalize(look);
    float3 right = cross(up, look);

	//
	// ���� ��ǥ �� �ﰢ�� �� �������� ����Ѵ�. 
	//
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
	
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);

	//
	// �簢�� ������ ���� �������� ��ȯ�ϰ� �ﰢ�� ��� ����մϴ�.
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
    // �ؽ�ó�� ���İ� 0.1���� ������ �ȼ��� ����Ѵ�.
    // ���̴� �ȿ��� �� ������ �ִ��� ���� �����ϴ� ���� �ٶ����ϴ�.
    // �׷��� �� �� ���̴��� ������ �ڵ��� ������ ������ �� �����Ƿ� ȿ�����̴�.
	clip(diffuseAlbedo.a - 0.1f);
#endif


    // ������ �����ϸ� ����ȭ���� ���� �� �����Ƿ� �ٽ� ����ȭ�մϴ�.
    pin.NormalW = normalize(pin.NormalW);

    // ������ ������ ����Ǵ� �����Դϴ�.
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;    // ����ȭ ����

    // ���� ��꿡 ���ԵǴ� �׵�.
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


