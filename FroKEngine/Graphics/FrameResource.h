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
	// 단위행렬을 저장한다.
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

// 이 버퍼는 하나의 랜더링 패스 전체에서 변하지 않는 상수 자료를 저장한다.
// 예로 들어 시점 위치나, 시야 행렬, 투영 행렬, 그리고 화면(렌더 대상) 크기에 관한 정보 등이 여기에 저장된다.
// 그 외 셰이더 프로그램에서 자주 사용되는 게임 시간 측정치 같은 정보 등이 이 버퍼에 저장된다.
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

	// 안개를 위한 변수
	DirectX::XMFLOAT4 FogColor = { 0.7f,0.7f,0.7f,1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	DirectX::XMFLOAT2 cbPerPassPad2;

	// 인덱스 [0, NUM_DIR_LIGHTS)는 방향 조명이다.
	// 인덱스 [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS)는 점광.
	// 인덱스 [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS) 
	// 개체당 최대 MaxLights에 대한 점적광.
	Light Lights[MAX_LIGHTS];
};

// CPU가 한 프레임의 커맨드 리스트들을 구축하는데 필요한 자원들을 대표하는 클래스이다.
// 응용 프로그램마다 필요한 자원이 다를 것이므로, 이런 클래스의 멤버 구성 역시 
// 응용 프로그램마다 달라야 할 것이다.
class FrameResource
{
public :
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt);
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT waveVertCount);
	FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT nMaterialCount, UINT waveVertCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	// 명령 할당자들을 GPU가 명령들을 다 처리한 다음에 재설정 해야한다.
	// 따라서 프레임마다 할당자가 필요하다.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCmdListAlloc;

	// 상수 버퍼는 그것을 참조하는 명령들을 GPU가 다 처리한 다음에
	// 갱신을 진행한다. 따라서 프레임마다 상수 버퍼를 새로 만들어야한다.
	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	// Fence는 현재 울타리 지점까지의 명령들을 표시하는 값이다.
	// 이 값은 GPU가 아직 이 프레임 자원들을 사용하고 있는가를 판정하는 용도이다.
	UINT64 nFence = 0;

	// GPU가 이를 참조하는 명령 처리를 완료할 때까지 동적 정점 버퍼를 업데이트할 수 없습니다.
	// 따라서 각 프레임에는 고유한 프레임이 필요합니다.
	std::unique_ptr<UploadBuffer<Vertex>> WavesVB = nullptr;
	
	// 주의 : 이 집합은 단순히 Material의 부분만 나타내는 집합이다.
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
};

