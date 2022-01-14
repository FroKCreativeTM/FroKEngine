#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ������ ������ ��ġ�� ���� ����ü
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
//	float3 iTex1 : TEXCOORD1)

struct ObjectConstants
{
	// ��������� �����Ѵ�.
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

	// ���� �ð� ��� ó����.
	virtual void Input(float fDeltaTime) override;
	virtual int Update(float fDeltaTime) override;
	// ������Ʈ�� ���� ���·� (�浹 ����) ��ó���� �ʿ��� ���
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

	// ���� Ŀ�ǵ� ����Ʈ�� �ʱ�ȭ �Ѵ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// �ʱ�ȭ ����� �����մϴ�.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ʱ�ȭ�� �Ϸ�� ������ ��ٸ��ϴ�.
	FlushCommandQueue();

	return true;
}

void FroKEngine::OnResize()
{
	Core::OnResize();

	// â ũ�Ⱑ �����Ǿ����Ƿ� ��Ⱦ�� ������Ʈ�ϰ� ���� ����� �ٽ� ����մϴ�
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

// ������ ���� �����Ѵ�.
// �̴� �ڿ��� ������ ���������ο� ���� �� ����� ���Դϴ�.
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

// ��� ���۸� �����մϴ�.
// 
// Input : void
// Output : void
inline void FroKEngine::BuildConstantBuffers()
{
	// ��ü n���� ��� �ڷḦ ���� �� �ִ� ���� �� �ִ� ��� �����Դϴ�.
	// ���⼭�� �ϳ��� �ڷḸ �ѱ�� �Ǵ� 1�� �־��ݴϴ�.
	m_ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 1, true);

	// 256����Ʈ ����� ����ϴ�.
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// ���� ��ü�� ���� �ּҸ� �����ɴϴ�.
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();

	// ���ۿ� �ִ� i��° ��ü ��� ���۷� �������մϴ�.
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_d3dDevice->CreateConstantBufferView(
		&cbvDesc,
		m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
}

// ��Ʈ ����� ������ ���̺��� �����մϴ�.
// 
// Input : void
// Output : void
inline void FroKEngine::BuildRootSignature()
{
	// ���̴� ���α׷��� �Ϲ������� �Է����� ���ҽ��� �ʿ�� �մϴ�(��� ����, �ؽ�ó, ���÷�).
	// ��Ʈ ������ ���̴� ���α׷��� ����ϴ� ���ҽ��� �����մϴ�.
	// ���̴� ���α׷��� �Լ���, �Է� ���ҽ��� �Լ� �Ű������� �����ϸ�
	// ��Ʈ ������ �Լ� ������ �����ϴ� ������ ������ �� �ֽ��ϴ�.

	// ��Ʈ �Ű������� ���̺�, ��Ʈ ������ �Ǵ� ��Ʈ ����� �� �ֽ��ϴ�.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV�� ���� ������ ���̺��� ����ϴ�.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// ���̺��� ������ ����
		0);	// �� ��Ʈ �Ű������� ���� ���̴� �μ����� ���� �������� ��ȣ( register (b0) )

	slotRootParameter[0].InitAsDescriptorTable(1,	// ����(range) ����
		&cbvTable);	// �������� �迭�� ����Ű�� ������

	// ��Ʈ ������ ��Ʈ �Ű������� �迭�Դϴ�.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// ���� ��� ���۷� ������ ����� ������ ����Ű�� ���� �������� ��Ʈ ������ ����ϴ�.
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

// ���̴��� ����Ʈ�ڵ�� �������ϰ�
// �Է� ���̾ƿ��� ���� ���̴��� �ѱ��.
inline void FroKEngine::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;
	
	// ���̴��� �������ؼ� ����Ʈ�ڵ�� ������.
	// �׸��� �� �ý����� GPU�� �°� ������ ����Ƽ�� ������� �������� �Ѵ�.
	m_vsByteCode = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

// �ڽ� ������Ʈ���� �����Ѵ�.
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

// ���������� ���� ��ü(Pipeline State Object)�� �����Ѵ�.
// ���⼭�� ���ݱ��� �Է� ���̾ƿ�, ����/�ȼ� ���̴��� ����� �����Ͷ����� ���¸� ������ ����
// ������ ���������ο� ��� �� ���¸� ������ �� �ִ� PSO�� �����մϴ�.
inline void FroKEngine::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	// �Է� ���̾ƿ��� �����ϴ�.
	psoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };

	// ��Ʈ ������ �����ϴ�.
	psoDesc.pRootSignature = m_RootSignature.Get();

	// ����/�ȼ� ���̴��� �����ϴ�.
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

	// �����Ͷ����� �κ��� ���̴��� ���� ���α׷����� �� ����
	// �ܼ��� ������ ������ �κ��̴�.
	// �����ͳ� �� �� �������� �����ݴϴ�.
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX; // ����ǥ��ȭ�� �����մϴ�.(���⼭�� �� � ǥ���� ��Ȱ��ȭ���� �ʴ� MAX�� �ֽ��ϴ�.)
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// �⺻ ������ �ﰢ���Դϴ�
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
		// �� �ȼ��� 4���� 1���� �ش��ϵ��� �մϴ�.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		// ���� ������ ī�޶� �˵��� ���� �Է��� ������� ������ ������Ʈ�մϴ�.
		m_Theta += dx;
		m_Phi += dy;

		// ���� mPhi�� �����մϴ�.
		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
	}

	if (GET_SINGLE(Input)->GetMouseRButton())
	{
		// ���콺 �� �ȼ� �̵��� ����� 0.005 ������ ������Ų��.
		float dx = 0.005f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - m_LastMousePos.y);

		// �Է¿� �����ؼ� ī�޶� �������� �����Ѵ�.
		m_Radius += dx - dy;

		// �������� �����Ѵ�.
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
	// ��ī��Ʈ ��ǥ��� ��ȯ�Ѵ�.
	float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	float y = m_Radius * cosf(m_Phi);

	// �� ��Ʈ������ �����Ѵ�.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);

	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX worldViewProj = world * view * proj;

	// ��� ���۸� �ֱ� worldViewProj ��ķ� ������Ʈ�Ѵ�.
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
	// Ŀ�ǵ� ��ϰ� ���õ� �޸𸮸� �����մϴ�.
	// ����� Ŀ�ǵ� ����Ʈ�� GPU���� ������ �Ϸ��� ��쿡�� �缳���� �� �ֽ��ϴ�.
	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	// Ŀ�ǵ� ����Ʈ�� ExecuteCommandList�� ���� ��� ��⿭�� �߰��� �� �缳���� �� �ֽ��ϴ�.
	// Ŀ�ǵ� ����Ʈ�� �����ϸ� �޸𸮰� ����˴ϴ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), m_PSO.Get()));

	// ����Ʈ�� Scissor Rect�� �����մϴ�. �̰��� Ŀ�ǵ� ����Ʈ�� �缳���� ������ �缳���Ǿ�� �մϴ�.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// ���ҽ� ��뷮�� ���� ���� ��ȯ�� ��Ÿ���ϴ�.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// �� ���ۿ� ���� ���۸� ����ϴ�.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// �������� ���۸� �����մϴ�.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// ��� ���� �� ������ ���� �����ɴϴ�.
	// �̴� ������ ���������ο� �ڿ��� ���� �� ����մϴ�.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// �׷��Ƚ� ��Ʈ ������ �����մϴ�.
	// SetGraphicsRootSignature�� �̿��ϸ� ������ ���̺��� �����ͼ� ���������ο� ���� �� �ֽ��ϴ�.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// �ڽ��� �׸��� ���� �Է� �����⸦ �����մϴ�.
	m_CommandList->IASetVertexBuffers(0, 1, &m_BoxGeo->VertexBufferView());
	m_CommandList->IASetIndexBuffer(&m_BoxGeo->IndexBufferView());
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// �׷��Ƚ� ��Ʈ ���� ���̺��� ���� �����ڸ� �����մϴ�.
	m_CommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

	// �ε����� ���缭 Ŀ�ǵ� ����Ʈ�� �� �ν��Ͻ��� �ѱ�ϴ�.
	m_CommandList->DrawIndexedInstanced(
		m_BoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// ���ҽ� ��뷮�� ���� ���� ��ȯ�� ��Ÿ���ϴ�.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// ���ڵ� Ŀ�ǵ带 �մϴ�.
	ThrowIfFailed(m_CommandList->Close());

	// ������ ��⿭�� Ŀ�ǵ� ����Ʈ�� �߰��մϴ�.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// back/front ���۸� �ٲߴϴ�.
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// ������ ����� �Ϸ�� ������ ��ٸ��ϴ�. 
	// �� ���� ��ȿ�����̸� �ܼ����� ���ؼ� �ִ� �ڵ��Դϴ�. ���߿� ������ �ڵ带 �����ϴ� ����� �����ݴϴ�.
	// ���� �����Ӵ� ��ٸ� �ʿ䰡 �����ϴ�.
	FlushCommandQueue();
}

