#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/FrameResource.h"
#include "Graphics/RenderItem.h"
#include "Graphics/GeometryGenerator.h"
#include "Graphics/Material.h"
#include "Graphics/Camera.h"
#include "Wave.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class WaveSimulator : public Core
{
public:
	WaveSimulator();
	~WaveSimulator();

	virtual bool Init(HINSTANCE hInstance, int nWidth = 800, int nHeight = 600) override;

private:

	// 전부 시간 기반 처리다.
	virtual void Input(float fDeltaTime) override;
	virtual int Update(float fDeltaTime) override;
	// 업데이트가 끝난 상태로 (충돌 전에) 후처리가 필요한 경우
	virtual int LateUpdate(float fDeltaTime) override;
	virtual void Collision(float fDeltaTime) override;
	virtual void Render(float fDeltaTime) override;

	virtual void OnResize()override;

	// 프레임 리소스를 빌드한다.
	void BuildFrameResources();

	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildWavesGeometryBuffers();
	void BuildMaterials();
	void BuildLandGeometry();
	void BuildRenderItems();
	void BuildPSO();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	void UpdateCamera(float fDeltaTime);
	void UpdateObjectCBs(float fDeltaTime);
	void UpdateMaterialCBs(float fDeltaTime);
	void UpdateMainPassCB(float fDeltaTime);
	void UpdateWaves(float fDeltaTime);

	// void MakeRenderItem();

	float GetHillsHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}

	XMFLOAT3 GetHillsNormal(float x, float z) const
	{
		// n = (-df/dx, 1, -df/dz)
		XMFLOAT3 n(
			-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
			1.0f,
			-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

		XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
		XMStoreFloat3(&n, unitNormal);

		return n;
	}

private:
	void OnMouseDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseUp(int x, int y);

private:
	ComPtr<ID3D12RootSignature>		m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>	m_CbvHeap = nullptr;

	UINT m_passCbvOffset = 0;

	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	// 셰이더들을 상수로 매핑해서 저장한다.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// 프레임 리소스를 저장하기 위한 vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	UINT m_CbvSrvDescriptorSize = 0;

	// 모든 렌더 아이템의 리스트.
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;

	// 파도에 대한 포인터
	std::unique_ptr<Waves> m_Waves;
	RenderItem* m_WaveRenderItem;

	// 마테리얼을 저장하기 위한 맵
	std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;

	// PSO 상태에 따라 달라지는 렌더링 아이템들이다.
	std::vector<RenderItem*> m_RenderitemLayer[(int)RenderLayer::Count];

	bool m_IsWireframe = false;

	// 패스 상수들을 저장해둔 구조체
	PassConstants m_tMainPassCB;

	// 지오메트리 정보를 저장하기 위한 map
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	XMFLOAT4X4	m_World = MathHelper::Identity4x4();
	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4	m_View = MathHelper::Identity4x4();
	XMFLOAT4X4	m_Proj = MathHelper::Identity4x4();

	// 태양(빛의 근원)을 컨트롤할 변수
	float m_fSunTheta = 1.25f * XM_PI;
	float m_fSunPhi = XM_PIDIV4;

	float		m_Theta = 1.5f * XM_PI;
	// float		m_Phi = XM_PIDIV4;
	float		m_Phi = 0.2f * XM_PI;
	float		m_Radius = 15.0f;

	// 카메라 클래스
	Camera m_Camera;

	POINT m_LastMousePos;
};

WaveSimulator::WaveSimulator() :
	Core() { }

WaveSimulator::~WaveSimulator() 
{ 
	if (m_d3dDevice != nullptr)
		FlushCommandQueue();
}

bool WaveSimulator::Init(HINSTANCE hInstance, int nWidth, int nHeight)
{
	if (!Core::Init(hInstance, nWidth, nHeight))
	{
		return false;
	}

	// 먼저 커맨드 리스트를 초기화 한다.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, so we have
	// to query this information.
	m_CbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 파도에 대한 설정을 한다.
	m_Waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	m_Camera.SetPosition(0.0f, 2.0f, -15.0f);

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSO();

	// 초기화 명령을 실행합니다.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 초기화가 완료될 때까지 기다립니다.
	FlushCommandQueue();

	return true;
}

void WaveSimulator::OnResize()
{
	Core::OnResize();

	// 창 크기가 조정되었으므로 종횡비를 업데이트하고 투영 행렬을 다시 계산합니다
	// XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	// XMStoreFloat4x4(&m_Proj, P);
	m_Camera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

inline void WaveSimulator::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), (UINT)m_Materials.size(), m_Waves->VertexCount()));
	}
}

// 서술자 힙을 생성한다.
// 이는 자원을 렌더링 파이프라인에 묶을 때 사용할 것입니다.
// Input : void
// Output : void
inline void WaveSimulator::BuildDescriptorHeaps()
{
	UINT objCnt = (UINT)m_OpaqueRenderItems.size();

	// 각 프레임 자원의 물체마다 하나씩 CBV 서술자가 필요하다.
	// +1은 각 프레임 자원에 필요한 패스별 CBV를 위한 것이다.
	UINT nDescriptors = (objCnt + 1) * gNumFrameResource;

	// 패스별 CBV의 시작 오프셋을 저장해둔다.
	// 이들은 마지막 세 서술자이다.
	m_passCbvOffset = objCnt * gNumFrameResource;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = nDescriptors;
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
inline void WaveSimulator::BuildConstantBufferViews()
{
	// 256바이트 배수로 맞춥니다.
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	UINT objCnt = (UINT)m_OpaqueRenderItems.size();

	// 각 프레임 자원의 물체마다 하나씩 CBV 서술자가 필요하다.
	for (int frameIdx = 0; frameIdx < gNumFrameResource; ++frameIdx)
	{
		auto objCB = m_frameResources[frameIdx]->ObjectCB->Resource();

		for (UINT i = 0; i < objCnt; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objCB->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋
			cbAddress += i * objCBByteSize;

			// 서술자 힙 안에서 cbv 오브젝트 오프셋을 구한다.
			int heapIdx = frameIdx * objCnt + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIdx, m_CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}


	UINT passCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// 마지막 3개의 서술자는 각 프레임 리소스를 위한 패스 상수 버퍼 뷰가 된다.
	for (int frameIndex = 0; frameIndex < gNumFrameResource; ++frameIndex)
	{
		auto passCB = m_frameResources[frameIndex]->PassCB->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// 서술자 힙 안의 패스 상수 버퍼 뷰를 위한 오프셋을 계산한다.
		int heapIndex = m_passCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

// 루트 서명과 서술자 테이블을 생성합니다.
// 
// Input : void
// Output : void
inline void WaveSimulator::BuildRootSignature()
{
	// 루트 매개변수는 테이블, 루트 설명자 또는 루트 상수일 수 있습니다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Create root CBV.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);

	// 루트 서명은 루트 매개변수의 배열입니다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 단일 상수 버퍼로 구성된 설명자 범위를 가리키는 단일 슬롯으로 루트 서명을 만듭니다.
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
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

// 셰이더을 바이트코드로 컴파일하고
// 입력 레이아웃을 만들어서 셰이더에 넘긴다.
inline void WaveSimulator::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	// 셰이더를 컴파일해서 바이트코드로 만들어낸다.
	// 그리고 그 시스템의 GPU에 맞게 최적의 네이티브 명령으로 컴파일을 한다.
	m_shaders["standardVS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\Default.hlsl", nullptr, "VS", "vs_5_0");
	m_shaders["opaquePS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\Default.hlsl", nullptr, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

inline void WaveSimulator::BuildWavesGeometryBuffers()
{
	std::vector<std::uint16_t> indices(3 * m_Waves->TriangleCount()); // 3 indices per face
	assert(m_Waves->VertexCount() < 0x0000ffff);

	// 각 쿼드마다 반복한다.
	int m = m_Waves->RowCount();
	int n = m_Waves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = m_Waves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	m_Geometries["waterGeo"] = std::move(geo);
}

inline void WaveSimulator::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->nMatCBIdx = 0;
	grass->DiffuseAlbedo = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->fRoughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->nMatCBIdx = 1;
	water->DiffuseAlbedo = XMFLOAT4(0.0f, 0.2f, 0.6f, 1.0f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->fRoughness = 0.0f;

	m_Materials["grass"] = std::move(grass);
	m_Materials["water"] = std::move(water);
}

inline void WaveSimulator::BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// 꼭짓점 요소를 추출하고 각 꼭짓점에 높이 함수를 적용합니다.
	// 또한 높이에 따라 정점에 색상을 지정하여 모래처럼 보이는 해변,
	// 풀이 무성한 낮은 언덕 및 눈 덮인 산봉우리를 만듭니다. 
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	m_Geometries["landGeo"] = std::move(geo);
}

inline void WaveSimulator::BuildRenderItems()
{
	auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->World = MathHelper::Identity4x4();
	wavesRitem->objCBIdx = 0;
	wavesRitem->Mat = m_Materials["water"].get();
	wavesRitem->pGeometry = m_Geometries["waterGeo"].get();
	wavesRitem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->nIdxCnt = wavesRitem->pGeometry->DrawArgs["grid"].IndexCount;
	wavesRitem->nStartIdxLocation = wavesRitem->pGeometry->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->nBaseVertexLocation = wavesRitem->pGeometry->DrawArgs["grid"].BaseVertexLocation;

	m_WaveRenderItem = wavesRitem.get();

	m_RenderitemLayer[(int)RenderLayer::Opaque].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->World = MathHelper::Identity4x4();
	gridRitem->objCBIdx = 1;
	gridRitem->Mat = m_Materials["grass"].get();
	gridRitem->pGeometry = m_Geometries["landGeo"].get();
	gridRitem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->nIdxCnt = gridRitem->pGeometry->DrawArgs["grid"].IndexCount;
	gridRitem->nStartIdxLocation = gridRitem->pGeometry->DrawArgs["grid"].StartIndexLocation;
	gridRitem->nBaseVertexLocation = gridRitem->pGeometry->DrawArgs["grid"].BaseVertexLocation;

	m_RenderitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	m_allRenderItems.push_back(std::move(wavesRitem));
	m_allRenderItems.push_back(std::move(gridRitem));
}

// 파이프라인 상태 객체(Pipeline State Object)를 생성한다.
// 여기서는 지금까지 입력 레이아웃, 정점/픽셀 셰이더를 만들고 래스터라이즈 상태를 설정한 것을
// 렌더링 파이프라인에 묶어서 이 상태를 제어할 수 있는 PSO를 생성합니다.
inline void WaveSimulator::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// 불투명체(Opaque) 상태의 렌더링
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

	//
	// 불투명한 와이어프레임 개체에 대한 PSO입니다.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque_wireframe"])));
}

inline void WaveSimulator::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_curFrameResource->ObjectCB->Resource();
	auto matCB = m_curFrameResource->MaterialCB->Resource();

	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->pGeometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->pGeometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->primitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +
			ri->objCBIdx * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + 
			ri->Mat->nMatCBIdx * matCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->nIdxCnt, 1, ri->nStartIdxLocation, ri->nBaseVertexLocation, 0);
	}
}

inline void WaveSimulator::OnMouseDown(int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

inline void WaveSimulator::OnMouseMove(int x, int y)
{
	if (GET_SINGLE(Input)->GetMouseLButton())
	{
		// 각 픽셀이 4분의 1도에 해당하도록 합니다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		m_Camera.Pitch(dy);
		m_Camera.RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

inline void WaveSimulator::OnMouseUp(int x, int y)
{
}

void WaveSimulator::Input(float fDeltaTime)
{
	if (GET_SINGLE(Input)->IsKeyDown(VK_ESCAPE))
	{
		m_bLoop = false;
		PostQuitMessage(0);
	}

	if (GET_SINGLE(Input)->IsKeyDown('1'))
	{
		cout << "1 pressed" << endl;
		m_IsWireframe = true;
	}
	else 
	{
		m_IsWireframe = false;
	}


	if (GET_SINGLE(Input)->IsKeyDown('W'))
	{
		m_Camera.Walk(20.0f * fDeltaTime);
	}
	if (GET_SINGLE(Input)->IsKeyDown('S'))
	{
		m_Camera.Walk(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(Input)->IsKeyDown('A'))
	{
		m_Camera.Strafe(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(Input)->IsKeyDown('D'))
	{
		m_Camera.Strafe(20.0f * fDeltaTime);
	}

	if (GET_SINGLE(Input)->IsKeyDown(VK_LEFT))
	{
		m_fSunTheta -= 1.0f * fDeltaTime;
	}
	if (GET_SINGLE(Input)->IsKeyDown(VK_RIGHT))
	{
		m_fSunTheta += 1.0f * fDeltaTime;
	}
	if (GET_SINGLE(Input)->IsKeyDown(VK_UP))
	{
		m_fSunPhi -= 1.0f * fDeltaTime;
	}
	if (GET_SINGLE(Input)->IsKeyDown(VK_DOWN))
	{
		m_fSunPhi += 1.0f * fDeltaTime;
	}
}

inline void WaveSimulator::UpdateCamera(float fDeltaTime)
{
	m_EyePos.x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	m_EyePos.z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	m_EyePos.y = m_Radius * cosf(m_Phi);

	XMVECTOR pos = XMVectorSet(m_EyePos.x, m_EyePos.y, m_EyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);
}

// 오브젝트의 상수 버퍼를 갱신한다.
inline void WaveSimulator::UpdateObjectCBs(float fDeltaTime)
{
	auto curObjectCB = m_curFrameResource->ObjectCB.get();

	// 상수들이 바뀌었을 때에만 cbuffer 자료를 갱신해야 한다.
	// 이러한 갱신을 프레임 자원마다 수행해야 한다.
	for (auto& e : m_allRenderItems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		curObjectCB->CopyData(e->objCBIdx, objConstants);

		// 다음 프레임 자원으로 넘어간다.
		e->nFramesDirty--;
	}
}

inline void WaveSimulator::UpdateMaterialCBs(float fDeltaTime)
{
	auto curMaterialCB = m_curFrameResource->MaterialCB.get();

	for (auto& e : m_Materials)
	{
		// 상수들이 변했을 때만 cbuffer를 갱신한다.
		// 이러한 갱신을 프레임 자원마다 수행해야 한다.
		Material* mat = e.second.get();

		if (mat->nFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConst.FresnelR0 = mat->FresnelR0;
			matConst.fRoughness = mat->fRoughness;

			curMaterialCB->CopyData(mat->nMatCBIdx, matConst);

			// 다음 프레임 자원으로 넘어간다.
			mat->nFramesDirty--;
		}
	}
}

inline void WaveSimulator::UpdateMainPassCB(float fDeltaTime)
{
	XMMATRIX view = m_Camera.GetView();
	XMMATRIX proj = m_Camera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_tMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_tMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_tMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_tMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_tMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_tMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_tMainPassCB.EyePosW = m_Camera.GetPosition3f();

	m_tMainPassCB.RenderTargetSize = XMFLOAT2((float)m_tRS.nWidth, (float)m_tRS.nHeight);
	m_tMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_tRS.nWidth, 1.0f / m_tRS.nHeight);

	m_tMainPassCB.NearZ = 1.0f;
	m_tMainPassCB.FarZ = 1000.0f;
	m_tMainPassCB.TotalTime = GET_SINGLE(Timer)->GetTotalTime();
	m_tMainPassCB.DeltaTime = fDeltaTime;

	m_tMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	XMVECTOR lightDir = -MathHelper::SphericalToCartesian(1.0f, m_fSunTheta, m_fSunPhi);

	XMStoreFloat3(&m_tMainPassCB.Lights[0].Direction, lightDir);
	m_tMainPassCB.Lights[0].Strength = { 1.0f, 1.0f, 0.9f };

	auto curPassCB = m_curFrameResource->PassCB.get();
	curPassCB->CopyData(0, m_tMainPassCB);
}

inline void WaveSimulator::UpdateWaves(float fDeltaTime)
{
	// 매 1/4초마다 무작위 웨이브를 생성합니다.
	static float t_base = 0.0f;
	if ((GET_SINGLE(Timer)->GetTotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, m_Waves->RowCount() - 5);
		int j = MathHelper::Rand(4, m_Waves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		m_Waves->Disturb(i, j, r);
	}

	// 웨이브 시뮬레이션을 업데이트합니다.
	m_Waves->Update(fDeltaTime);

	// 새 솔루션으로 웨이브 정점 버퍼를 업데이트한다.
	auto currWavesVB = m_curFrameResource->WavesVB.get();
	for (int i = 0; i < m_Waves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = m_Waves->Position(i);
		v.Normal = m_Waves->Normal(i);

		currWavesVB->CopyData(i, v);
	}

	// 파도 렌더 아이템의 동적 VB를 현재 프레임 VB로 설정합니다.
	m_WaveRenderItem->pGeometry->VertexBufferGPU = currWavesVB->Resource();
}

int WaveSimulator::Update(float fDeltaTime)
{
	m_Camera.UpdateViewMatrix();

	m_fSunPhi = MathHelper::Clamp(m_fSunPhi, 1.0f, XM_PIDIV2);

	// 원형 프레임 리소스 배열을 순환한다.
	m_nCurFrameResourceIdx = (m_nCurFrameResourceIdx + 1) % gNumFrameResource;
	m_curFrameResource = m_frameResources[m_nCurFrameResourceIdx].get();

	// GPU가 현재 프레임 리소스의 명령 처리를 완료했는가를 판별한다
	// 렇지 않은 경우 GPU가 이 울타리 지점까지 명령을 완료할 때까지 기다린다.
	if (m_curFrameResource->nFence != 0 && m_Fence->GetCompletedValue() < m_curFrameResource->nFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_curFrameResource->nFence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs(fDeltaTime);
	UpdateMaterialCBs(fDeltaTime);
	UpdateMainPassCB(fDeltaTime);
	UpdateWaves(fDeltaTime);

	return 0;
}

int WaveSimulator::LateUpdate(float fDeltaTime)
{
	return 0;
}

void WaveSimulator::Collision(float fDeltaTime)
{
}

void WaveSimulator::Render(float fDeltaTime)
{
	// 먼저 커맨드 리스트 할당자를 가져온다.
	auto cmdListAlloc = m_curFrameResource->pCmdListAlloc;

	// 커맨드 기록과 관련된 메모리를 재사용합니다.
	// 연결된 커맨드 리스트가 GPU에서 실행을 완료한 경우에만 재설정할 수 있습니다.
	ThrowIfFailed(cmdListAlloc->Reset());

	// 커맨드 리스트는 ExecuteCommandList를 통해 명령 대기열에 추가된 후 재설정할 수 있습니다.
	// 커맨드 리스트를 재사용하면 메모리가 재사용됩니다.
	// 기본값으로 와이어 프레임이 켜져 있습니다.
	if (m_IsWireframe)
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque_wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));
	}

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

	// 그래픽스 루트 서명을 설정합니다.
	// SetGraphicsRootSignature을 이용하면 서술자 테이블을 가져와서 파이프라인에 묶을 수 있습니다.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// 패스당 상수 버퍼를 바인딩합니다. 이 작업은 패스당 한 번만 수행하면 됩니다.
	auto passCB = m_curFrameResource->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Opaque]);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 리코딩 명령 완료
	ThrowIfFailed(m_CommandList->Close());

	// 실행할 대기열에 명령 목록을 추가합니다.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 후면 및 전면 버퍼 교체
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// 이 펜스 포인트까지 명령을 표시하려면 펜스 값을 증가시킨다.
	m_curFrameResource->nFence = ++m_CurrentFence;

	// 명령 대기열에 명령을 추가하여 새 펜스 포인트를 설정한다.
	// GPU 타임라인에 있으므로 GPU가 이 Signal() 이전의 모든 명령 처리를 완료할 때까지 새 펜스 포인트가 설정되지 않는다.
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

