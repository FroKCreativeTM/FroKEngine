#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/FrameResource.h"
#include "Graphics/RenderItem.h"
#include "Graphics/GeometryGenerator.h"
#include "Graphics/Material.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture/Texture.h"
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

	// 이 부분은 레벨들을 레이어화하면 해결될 부분인 것 같다.
	void LoadTexture();
	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildWavesGeometryBuffers();
	void BuildMaterials();
	void BuildLandGeometry();
	void BuildBoxGeometry();
	void BuildRenderItems();
	void BuildPSO();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	void AnimateMaterials(float fDeltaTime);
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
	ComPtr<ID3D12DescriptorHeap>	m_SrvHeap = nullptr;

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

	// 텍스처를 저장하기 위한 맵
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;

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

	// 이 힙 유형에서 설명자의 증분 크기를 가져옵니다. 
	// 이것은 하드웨어에 따라 다르므로 이 정보를 쿼리해야 합니다.
	m_CbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 파도에 대한 설정을 한다.
	m_Waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	m_Camera.SetPosition(0.0f, 2.0f, -15.0f);

	LoadTexture();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildBoxGeometry();
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

inline void WaveSimulator::LoadTexture()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->strName = "grassTex";
	grassTex->strFileName = L"Graphics/Texture/Datas/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), grassTex->strFileName.c_str(),
		grassTex->pResource, grassTex->pUploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->strName = "waterTex";
	waterTex->strFileName = L"Graphics/Texture/Datas/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), waterTex->strFileName.c_str(),
		waterTex->pResource, waterTex->pUploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->strName = "fenceTex";
	fenceTex->strFileName = L"Graphics/Texture/Datas/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), fenceTex->strFileName.c_str(),
		fenceTex->pResource, fenceTex->pUploadHeap));

	m_Textures[grassTex->strName] = std::move(grassTex);
	m_Textures[waterTex->strName] = std::move(waterTex);
	m_Textures[fenceTex->strName] = std::move(fenceTex);
}

// 서술자 힙을 생성한다.
// 이는 자원을 렌더링 파이프라인에 묶을 때 사용할 것입니다.
// Input : void
// Output : void
inline void WaveSimulator::BuildDescriptorHeaps()
{
	//
	// SRV 힙을 생성한다.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)));

	//
	// 힙이 할 일을 적어낸다.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = m_Textures["grassTex"]->pResource;
	auto waterTex = m_Textures["waterTex"]->pResource;
	auto fenceTex = m_Textures["fenceTex"]->pResource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	m_d3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// 다음 서술자를 가져온다.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// 다음 서술자를 가져온다.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);
}

// 루트 서명과 서술자 테이블을 생성합니다.
// 
// Input : void
// Output : void
inline void WaveSimulator::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// 루트 매개변수는 테이블, 루트 설명자 또는 루트 상수일 수 있습니다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// 성능 팁: 가장 자주 발생하는 것에서 가장 적게 발생하는 순서로 정렬하십시오.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// 루트 서명은 루트 매개변수의 배열입니다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	// D3D_SHADER_MACRO 구조체를 이용해서 셰이더 파이프라인에 매개변수를 넘겨줄 수 있다.
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	// 셰이더를 컴파일해서 바이트코드로 만들어낸다.
	// 그리고 그 시스템의 GPU에 맞게 최적의 네이티브 명령으로 컴파일을 한다.
	m_shaders["standardVS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\BlendDefault.hlsl", nullptr, "VS", "vs_5_1");
	// 얘는 왜 맨날 에러를 뿜을까
	// 아 현타온다.
	m_shaders["opaquePS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\BlendDefault.hlsl", defines, "PS", "ps_5_1");
	m_shaders["alphaTestedPS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\BlendDefault.hlsl", alphaTestDefines, "PS", "ps_5_1");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	grass->nDiffuseSrvHeapIdx = 0;
	grass->DiffuseAlbedo = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->fRoughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->nMatCBIdx = 1;
	water->nDiffuseSrvHeapIdx = 1;
	water->DiffuseAlbedo = XMFLOAT4(0.0f, 0.2f, 0.6f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->fRoughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->nMatCBIdx = 2;
	wirefence->nDiffuseSrvHeapIdx = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence->fRoughness = 0.25f;

	m_Materials["grass"] = std::move(grass);
	m_Materials["water"] = std::move(water);
	m_Materials["wirefence"] = std::move(wirefence);
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
		vertices[i].TexC = grid.Vertices[i].TexC;
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

inline void WaveSimulator::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

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

	geo->DrawArgs["box"] = submesh;

	m_Geometries["boxGeo"] = std::move(geo);
}

inline void WaveSimulator::BuildRenderItems()
{
	auto wavesRitem = std::make_unique<RenderItem>();
	wavesRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->objCBIdx = 0;
	wavesRitem->Mat = m_Materials["water"].get();
	wavesRitem->pGeometry = m_Geometries["waterGeo"].get();
	wavesRitem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->nIdxCnt = wavesRitem->pGeometry->DrawArgs["grid"].IndexCount;
	wavesRitem->nStartIdxLocation = wavesRitem->pGeometry->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->nBaseVertexLocation = wavesRitem->pGeometry->DrawArgs["grid"].BaseVertexLocation;

	m_WaveRenderItem = wavesRitem.get();
	m_RenderitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->objCBIdx = 1;
	gridRitem->Mat = m_Materials["grass"].get();
	gridRitem->pGeometry = m_Geometries["landGeo"].get();
	gridRitem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->nIdxCnt = gridRitem->pGeometry->DrawArgs["grid"].IndexCount;
	gridRitem->nStartIdxLocation = gridRitem->pGeometry->DrawArgs["grid"].StartIndexLocation;
	gridRitem->nBaseVertexLocation = gridRitem->pGeometry->DrawArgs["grid"].BaseVertexLocation;

	m_RenderitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRitem->objCBIdx = 2;
	boxRitem->Mat = m_Materials["wirefence"].get();
	boxRitem->pGeometry = m_Geometries["boxGeo"].get();
	boxRitem->primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->nIdxCnt = boxRitem->pGeometry->DrawArgs["box"].IndexCount;
	boxRitem->nStartIdxLocation = boxRitem->pGeometry->DrawArgs["box"].StartIndexLocation;
	boxRitem->nBaseVertexLocation = boxRitem->pGeometry->DrawArgs["box"].BaseVertexLocation;

	m_RenderitemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem.get());

	m_allRenderItems.push_back(std::move(wavesRitem));
	m_allRenderItems.push_back(std::move(gridRitem));
	m_allRenderItems.push_back(std::move(boxRitem));
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPSODesc = opaquePsoDesc;

	// 블렌딩을 위한 코드
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;		// 합 연산
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPSODesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(
		&transparentPSODesc, IID_PPV_ARGS(&m_PSOs["transparent"])));

	// 알파 판정을 위한 PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestPSODesc = opaquePsoDesc;
	alphaTestPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["alphaTestedPS"]->GetBufferPointer()),
		m_shaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(
		&alphaTestPSODesc, IID_PPV_ARGS(&m_PSOs["alphaTested"])));

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

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->nDiffuseSrvHeapIdx, m_CbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +
			ri->objCBIdx * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + 
			ri->Mat->nMatCBIdx * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->nIdxCnt, 1, ri->nStartIdxLocation, ri->nBaseVertexLocation, 0);
	}
}

inline std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> WaveSimulator::GetStaticSamplers()
{
	// 애플리케이션에는 일반적으로 소수의 샘플러만 필요합니다. 
	// 따라서 그것들을 모두 미리 정의하고 루트 서명의 일부로 사용할 수 있도록 유지하는 것이 좋습니다.

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
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

inline void WaveSimulator::AnimateMaterials(float fDeltaTime)
{
	// Scroll the water material texture coordinates.
	auto waterMat = m_Materials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * fDeltaTime;
	tv += 0.02f * fDeltaTime;

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// 재질이 변경되었으므로 cbuffer를 업데이트해야 합니다.
	waterMat->nFramesDirty = gNumFrameResource;
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
		if (e->nFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			curObjectCB->CopyData(e->objCBIdx, objConstants);

			// 다음 프레임 자원으로 넘어간다.
			e->nFramesDirty--;
		}
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
			XMStoreFloat4x4(&matConst.MatTransform, XMMatrixTranspose(matTransform));

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

	// m_tMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };

	XMVECTOR lightDir = -MathHelper::SphericalToCartesian(1.0f, m_fSunTheta, m_fSunPhi);
	XMStoreFloat3(&m_tMainPassCB.Lights[0].Direction, lightDir);
	m_tMainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.9f };
	m_tMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_tMainPassCB.Lights[1].Strength = { 0.5f, 0.5f, 0.5f };
	m_tMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_tMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

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

		// [-w/2,w/2] --> [0,1] 매핑을 통해 위치에서 tex-coords 파생
		v.TexC.x = 0.5f + v.Pos.x / m_Waves->Width();
		v.TexC.y = 0.5f - v.Pos.z / m_Waves->Depth();

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

	AnimateMaterials(fDeltaTime);
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
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&m_tMainPassCB.FogColor, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 렌더링할 버퍼를 지정합니다.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 서술자 힙을 가져와서 이를 설정한다.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 그래픽스 루트 서명을 설정합니다.
	// SetGraphicsRootSignature을 이용하면 서술자 테이블을 가져와서 파이프라인에 묶을 수 있습니다.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// 패스당 상수 버퍼를 바인딩합니다. 이 작업은 패스당 한 번만 수행하면 됩니다.
	auto passCB = m_curFrameResource->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Opaque]);

	m_CommandList->SetPipelineState(m_PSOs["alphaTested"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::AlphaTested]);

	m_CommandList->SetPipelineState(m_PSOs["transparent"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Transparent]);

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