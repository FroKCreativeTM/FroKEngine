#pragma once

#include "../MathHelper.h"
#include "UploadBuffer.h"
#include "Material.h"

using namespace std;
using namespace DirectX;

const int MAX_LIGHTS = 16;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
};

struct ObjectConstants
{
	// ��������� �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

// �� ���۴� �ϳ��� ������ �н� ��ü���� ������ �ʴ� ��� �ڷḦ �����Ѵ�.
// ���� ��� ���� ��ġ��, �þ� ���, ���� ���, �׸��� ȭ��(���� ���) ũ�⿡ ���� ���� ���� ���⿡ ����ȴ�.
// �� �� ���̴� ���α׷����� ���� ���Ǵ� ���� �ð� ����ġ ���� ���� ���� �� ���ۿ� ����ȴ�.
struct PassConstants
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// �Ȱ��� ���� ����
	DirectX::XMFLOAT4 FogColor = { 0.7f,0.7f,0.7f,1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	DirectX::XMFLOAT2 cbPerPassPad2;

	// �ε��� [0, NUM_DIR_LIGHTS)�� ���� �����̴�.
	// �ε��� [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS)�� ����.
	// �ε��� [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS) 
	// ��ü�� �ִ� MaxLights�� ���� ������.
	Light Lights[MAX_LIGHTS];
};

// CPU�� �� �������� Ŀ�ǵ� ����Ʈ���� �����ϴµ� �ʿ��� �ڿ����� ��ǥ�ϴ� Ŭ�����̴�.
// ���� ���α׷����� �ʿ��� �ڿ��� �ٸ� ���̹Ƿ�, �̷� Ŭ������ ��� ���� ���� 
// ���� ���α׷����� �޶�� �� ���̴�.
class FrameResource
{
public :
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt);
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT waveVertCount);
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT nMaterialCount, UINT waveVertCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	// ��� �Ҵ��ڵ��� GPU�� ��ɵ��� �� ó���� ������ �缳�� �ؾ��Ѵ�.
	// ���� �����Ӹ��� �Ҵ��ڰ� �ʿ��ϴ�.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCmdListAlloc;

	// ��� ���۴� �װ��� �����ϴ� ��ɵ��� GPU�� �� ó���� ������
	// ������ �����Ѵ�. ���� �����Ӹ��� ��� ���۸� ���� �������Ѵ�.
	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	// Fence�� ���� ��Ÿ�� ���������� ��ɵ��� ǥ���ϴ� ���̴�.
	// �� ���� GPU�� ���� �� ������ �ڿ����� ����ϰ� �ִ°��� �����ϴ� �뵵�̴�.
	UINT64 nFence = 0;

	// GPU�� �̸� �����ϴ� ��� ó���� �Ϸ��� ������ ���� ���� ���۸� ������Ʈ�� �� �����ϴ�.
	// ���� �� �����ӿ��� ������ �������� �ʿ��մϴ�.
	std::unique_ptr<UploadBuffer<Vertex>> WavesVB = nullptr;
	
	// ���� : �� ������ �ܼ��� Material�� �κи� ��Ÿ���� �����̴�.
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
};

