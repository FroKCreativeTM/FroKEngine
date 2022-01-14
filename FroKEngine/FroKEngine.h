#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// 법선과 색상의 위치를 가진 구조체
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	// XMFLOAT2 tex0;
	// XMFLOAT2 tex1;
};

// 위의 데이터는 D3D12_INPUT_ELEMENT_DESC 배열을 이용해서 입력 배치를 서술한다
// 예로 들어 위의 경우는(주석까지 포함해서) 다음과 같이 표현한다.
// D3D12_INPUT_ELEMENT_DESC vertexDesc[] = 
// {
//		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"COLOR", 0, DXGI_FORMAT_A32R32G32B32_FLOAT, 0, 12, // 12는 offset
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,	// 12 + 16
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//		{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36,	// 12 + 16 + 8
//			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
// };
// 그리고 위 코드는 Vertex Shader 코드에서 다음과 같이 입력을 받는다.
// VertexOut VS(float3 iPos : POSITION,
//	float3 iNormal : NORMAL,
//	float3 iTex0 : TEXCOORD0,
//	float3 iTex1 : TEXCOORD1)

struct ObjectConstants
{
	// 단위행렬을 저장한다.
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	// float gTime = 0.0f;
};

class FroKEngine : public Core
{
public:
	FroKEngine();
	~FroKEngine();

	virtual bool Init(HINSTANCE hInstance, int nWidth = 1280, int nHeight = 720) override;

private:

	// 전부 시간 기반 처리다.
	virtual void Input(float fDeltaTime) override;
	virtual int Update(float fDeltaTime) override;
	// 업데이트가 끝난 상태로 (충돌 전에) 후처리가 필요한 경우
	virtual int LateUpdate(float fDeltaTime) override;
	virtual void Collision(float fDeltaTime) override;
	virtual void Render(float fDeltaTime) override;

	virtual void OnResize()override;

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

private :
	void OnMouseDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseUp(int x, int y);

private :
	ComPtr<ID3D12RootSignature>		m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>	m_CbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	ComPtr<ID3D12PipelineState> m_PSO = nullptr;

	XMFLOAT4X4	m_World	= MathHelper::Identity4x4();
	XMFLOAT4X4	m_View	= MathHelper::Identity4x4();
	XMFLOAT4X4	m_Proj	= MathHelper::Identity4x4();

	float		m_Theta = 1.5f * XM_PI;
	float		m_Phi = XM_PIDIV4;
	float		m_Radius = 5.0f;

	POINT m_LastMousePos;
};

FroKEngine::FroKEngine() : 
	Core() { }
FroKEngine::~FroKEngine() { }

bool FroKEngine::Init(HINSTANCE hInstance, int nWidth, int nHeight)
{
	if (!Core::Init(hInstance, nWidth, nHeight))
	{
		return false;
	}

	// 먼저 커맨드 리스트를 초기화 한다.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// 초기화 명령을 실행합니다.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 초기화가 완료될 때까지 기다립니다.
	FlushCommandQueue();

	return true;
}

void FroKEngine::OnResize()
{
	Core::OnResize();

	// 창 크기가 조정되었으므로 종횡비를 업데이트하고 투영 행렬을 다시 계산합니다
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

// 서술자 힙을 생성한다.
// 이는 자원을 렌더링 파이프라인에 묶을 때 사용할 것입니다.
// Input : void
// Output : void
inline void FroKEngine::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_CbvHeap)));
}

// 상수 버퍼를 빌드합니다.
// 
// Input : void
// Output : void
inline void FroKEngine::BuildConstantBuffers()
{
	// 물체 n개의 상수 자료를 담을 수 있는 담을 수 있는 상수 버퍼입니다.
	// 여기서는 하나의 자료만 넘기면 되니 1을 넣어줍니다.
	m_ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 1, true);

	// 256바이트 배수로 맞춥니다.
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// 버퍼 자체의 시작 주소를 가져옵니다.
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();

	// 버퍼에 있는 i번째 객체 상수 버퍼로 오프셋합니다.
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_d3dDevice->CreateConstantBufferView(
		&cbvDesc,
		m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
}

// 루트 서명과 서술자 테이블을 생성합니다.
// 
// Input : void
// Output : void
inline void FroKEngine::BuildRootSignature()
{
	// 셰이더 프로그램은 일반적으로 입력으로 리소스를 필요로 합니다(상수 버퍼, 텍스처, 샘플러).
	// 루트 서명은 셰이더 프로그램이 기대하는 리소스를 정의합니다.
	// 셰이더 프로그램을 함수로, 입력 리소스를 함수 매개변수로 생각하면
	// 루트 서명은 함수 서명을 정의하는 것으로 생각할 수 있습니다.

	// 루트 매개변수는 테이블, 루트 설명자 또는 루트 상수일 수 있습니다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV의 단일 서술자 테이블을 만듭니다.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// 테이블의 서술자 갯수
		0);	// 이 루트 매개변수에 묶일 셰이더 인수들의 기준 레지스터 번호( register (b0) )

	slotRootParameter[0].InitAsDescriptorTable(1,	// 구간(range) 갯수
		&cbvTable);	// 구간들의 배열을 가리키는 포인터

	// 루트 서명은 루트 매개변수의 배열입니다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 단일 상수 버퍼로 구성된 사술자 구간을 가리키는 단일 슬롯으로 루트 서명을 만듭니다.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSignature)));
}

// 셰이더을 바이트코드로 컴파일하고
// 입력 레이아웃을 만들어서 셰이더에 넘긴다.
inline void FroKEngine::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;
	
	// 셰이더를 컴파일해서 바이트코드로 만들어낸다.
	// 그리고 그 시스템의 GPU에 맞게 최적의 네이티브 명령으로 컴파일을 한다.
	m_vsByteCode = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

// 박스 지오메트리를 생성한다.
inline void FroKEngine::BuildBoxGeometry()
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_BoxGeo = std::make_unique<MeshGeometry>();
	m_BoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_BoxGeo->VertexBufferCPU));
	CopyMemory(m_BoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_BoxGeo->IndexBufferCPU));
	CopyMemory(m_BoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_BoxGeo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, m_BoxGeo->VertexBufferUploader);

	m_BoxGeo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, m_BoxGeo->IndexBufferUploader);

	m_BoxGeo->VertexByteStride = sizeof(Vertex);
	m_BoxGeo->VertexBufferByteSize = vbByteSize;
	m_BoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_BoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_BoxGeo->DrawArgs["box"] = submesh;
}

// 파이프라인 상태 객체(Pipeline State Object)를 생성한다.
// 여기서는 지금까지 입력 레이아웃, 정점/픽셀 셰이더를 만들고 래스터라이즈 상태를 설정한 것을
// 렌더링 파이프라인에 묶어서 이 상태를 제어할 수 있는 PSO를 생성합니다.
inline void FroKEngine::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	// 입력 레이아웃을 묶습니다.
	psoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };

	// 루트 서명을 묶습니다.
	psoDesc.pRootSignature = m_RootSignature.Get();

	// 정점/픽셀 셰이더를 묶습니다.
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};

	// 래스터라이즈 부분은 셰이더를 직접 프로그래밍할 수 없고
	// 단순히 설정만 가능한 부분이다.
	// 레스터나 그 외 설정들을 묶어줍니다.
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX; // 다중표본화를 설정합니다.(여기서는 그 어떤 표본도 비활성화하지 않는 MAX를 넣습니다.)
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 기본 도형은 삼각형입니다
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_BackBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_DepthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO)));
}

inline void FroKEngine::OnMouseDown(int x, int y)
{

}

inline void FroKEngine::OnMouseMove(int x, int y)
{
	if (GET_SINGLE(Input)->GetMouseLButton())
	{
		// 각 픽셀이 4분의 1도에 해당하도록 합니다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		// 상자 주위의 카메라 궤도에 대한 입력을 기반으로 각도를 업데이트합니다.
		m_Theta += dx;
		m_Phi += dy;

		// 각도 mPhi를 제한합니다.
		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
	}

	if (GET_SINGLE(Input)->GetMouseRButton())
	{
		// 마우스 한 픽셀 이동을 장면의 0.005 단위에 대응시킨다.
		float dx = 0.005f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - m_LastMousePos.y);

		// 입력에 기초해서 카메라 반지름을 갱신한다.
		m_Radius += dx - dy;

		// 반지름을 제한한다.
		m_Radius = MathHelper::Clamp(m_Radius, 3.0f, 15.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

inline void FroKEngine::OnMouseUp(int x, int y)
{
}

void FroKEngine::Input(float fDeltaTime)
{
	if (GET_SINGLE(Input)->IsKeyDown(VK_ESCAPE))
	{
		m_bLoop = false;
		PostQuitMessage(0);
	}
}

int FroKEngine::Update(float fDeltaTime)
{
	// 데카르트 좌표계로 변환한다.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	// 뷰 매트릭스를 생성한다.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);

	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX worldViewProj = world * view * proj;

	// 상수 버퍼를 최근 worldViewProj 행렬로 업데이트한다.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	// objConstants.gTime = GET_SINGLE(Timer)->GetTotalTime();
	m_ObjectCB->CopyData(0, objConstants);

	return 0;
}

int FroKEngine::LateUpdate(float fDeltaTime)
{
	return 0;
}

void FroKEngine::Collision(float fDeltaTime)
{
}

void FroKEngine::Render(float fDeltaTime)
{
	// 커맨드 기록과 관련된 메모리를 재사용합니다.
	// 연결된 커맨드 리스트가 GPU에서 실행을 완료한 경우에만 재설정할 수 있습니다.
	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	// 커맨드 리스트는 ExecuteCommandList를 통해 명령 대기열에 추가된 후 재설정할 수 있습니다.
	// 커맨드 리스트를 재사용하면 메모리가 재사용됩니다.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), m_PSO.Get()));

	// 뷰포트와 Scissor Rect를 설정합니다. 이것은 커맨드 리스트가 재설정될 때마다 재설정되어야 합니다.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// 리소스 사용량에 대한 상태 전환을 나타냅니다.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 백 버퍼와 깊이 버퍼를 지웁니다.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 렌더링할 버퍼를 지정합니다.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 상수 버퍼 뷰 서술자 힙을 가져옵니다.
	// 이는 렌더링 파이프라인에 자원을 묶을 때 사용합니다.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 그래픽스 루트 서명을 설정합니다.
	// SetGraphicsRootSignature을 이용하면 서술자 테이블을 가져와서 파이프라인에 묶을 수 있습니다.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// 박스를 그리기 위한 입력 조립기를 설정합니다.
	m_CommandList->IASetVertexBuffers(0, 1, &m_BoxGeo->VertexBufferView());
	m_CommandList->IASetIndexBuffer(&m_BoxGeo->IndexBufferView());
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 그래픽스 루트 서명 테이블의 대한 서술자를 설정합니다.
	m_CommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

	// 인덱스에 맞춰서 커맨드 리스트에 이 인스턴스를 넘깁니다.
	m_CommandList->DrawIndexedInstanced(
		m_BoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// 리소스 사용량에 대한 상태 전환을 나타냅니다.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 리코딩 커맨드를 합니다.
	ThrowIfFailed(m_CommandList->Close());

	// 실행할 대기열에 커맨드 리스트를 추가합니다.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// back/front 버퍼를 바꿉니다.
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// 프레임 명령이 완료될 때까지 기다립니다. 
	// 이 대기는 비효율적이며 단순성을 위해서 있는 코드입니다. 나중에 렌더링 코드를 구성하는 방법을 보여줍니다.
	// 따라서 프레임당 기다릴 필요가 없습니다.
	FlushCommandQueue();
}

