#pragma once

#include "../MathHelper.h"
#include "UploadBuffer.h"

using namespace std;
using namespace DirectX;

// Vertex ����
/*// ������ ������ ��ġ�� ���� ����ü
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	// XMFLOAT2 tex0;
	// XMFLOAT2 tex1;
};

// ���� �����ʹ� D3D12_INPUT_ELEMENT_DESC �迭�� �̿��ؼ� �Է� ��ġ�� �����Ѵ�
// ���� ��� ���� ����(�ּ����� �����ؼ�) ������ ���� ǥ���Ѵ�.
// D3D12_INPUT_ELEMENT_DESC vertexDesc[] = 
// {
//		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"COLOR", 0, DXGI_FORMAT_A32R32G32B32_FLOAT, 0, 12, // 12�� offset
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,	// 12 + 16
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36,	// 12 + 16 + 8
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
// };
// �׸��� �� �ڵ�� Vertex Shader �ڵ忡�� ������ ���� �Է��� �޴´�.
// VertexOut VS(float3 iPos : POSITION,
//	float3 iNormal : NORMAL,
//	float3 iTex0 : TEXCOORD0,
//	float3 iTex1 : TEXCOORD1)*/

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectConstants
{
	// ��������� �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();
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
};

// CPU�� �� �������� Ŀ�ǵ� ����Ʈ���� �����ϴµ� �ʿ��� �ڿ����� ��ǥ�ϴ� Ŭ�����̴�.
// ���� ���α׷����� �ʿ��� �ڿ��� �ٸ� ���̹Ƿ�, �̷� Ŭ������ ��� ���� ���� 
// ���� ���α׷����� �޶�� �� ���̴�.
class FrameResource
{
public : 
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt);
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

};

