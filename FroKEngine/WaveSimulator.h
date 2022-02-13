#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/FrameResource.h"
#include "Graphics/RenderItem.h"
#include "Object/StaticObj/MeshObject.h"
#include "Graphics/GeometryGenerator.h"
#include "Graphics/Material.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture/Texture.h"
#include "Wave.h"
#include "TreeSpriteVertex.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class WaveSimulator : public Core
{
public:
	WaveSimulator();
	~WaveSimulator();

	virtual bool Init(HINSTANCE hInstance, int nWidth = 800, int nHeight = 600) override;

private:

	// ���� �ð� ��� ó����.
	virtual void Input(float fDeltaTime) override;
	virtual int Update(float fDeltaTime) override;
	// ������Ʈ�� ���� ���·� (�浹 ����) ��ó���� �ʿ��� ���
	virtual int LateUpdate(float fDeltaTime) override;
	virtual void Collision(float fDeltaTime) override;
	virtual void Render(float fDeltaTime) override;

	virtual void OnResize()override;

	// ������ ���ҽ��� �����Ѵ�.
	void BuildFrameResources();

	// �� �κ��� �������� ���̾�ȭ�ϸ� �ذ�� �κ��� �� ����.
	void LoadTexture();
	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildWavesGeometryBuffers();
	void BuildMaterials();
	void BuildLandGeometry();
	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
	void BuildRenderItems();
	void BuildPSO();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<MeshObject*>& ritems, float fDeltaTime);

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

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
	// ���� ���̴��� �Ѱ��� 
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_TreeSpriteInputLayout;

	// ������ ���ҽ��� �����ϱ� ���� vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	UINT m_CbvSrvDescriptorSize = 0;

	// ��� ���� �������� ����Ʈ.
	std::vector<std::unique_ptr<Object>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;

	// �ĵ��� ���� ������
	std::unique_ptr<Waves> m_Waves;
	MeshObject* m_WaveRenderItem;

	// ���׸����� �����ϱ� ���� ��
	std::unordered_map<std::string, Material*> m_Materials;

	// PSO ���¿� ���� �޶����� ������ �����۵��̴�.
	std::vector<MeshObject*> m_RenderitemLayer[(int)RenderLayer::Count];

	bool m_IsWireframe = false;

	// �н� ������� �����ص� ����ü
	PassConstants m_tMainPassCB;

	// ������Ʈ�� ������ �����ϱ� ���� map
	std::unordered_map<std::string, MeshGeometry*> m_Geometries;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	// �ؽ�ó�� �����ϱ� ���� ��
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;

	XMFLOAT4X4	m_World = MathHelper::Identity4x4();
	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4	m_View = MathHelper::Identity4x4();
	XMFLOAT4X4	m_Proj = MathHelper::Identity4x4();

	// �¾�(���� �ٿ�)�� ��Ʈ���� ����
	float m_fSunTheta = 1.25f * XM_PI;
	float m_fSunPhi = XM_PIDIV4;

	float		m_Theta = 1.5f * XM_PI;
	// float		m_Phi = XM_PIDIV4;
	float		m_Phi = 0.2f * XM_PI;
	float		m_Radius = 15.0f;

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

	// ���� Ŀ�ǵ� ����Ʈ�� �ʱ�ȭ �Ѵ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// �� �� �������� �������� ���� ũ�⸦ �����ɴϴ�. 
	// �̰��� �ϵ��� ���� �ٸ��Ƿ� �� ������ �����ؾ� �մϴ�.
	m_CbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// �ĵ��� ���� ������ �Ѵ�.
	m_Waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	// ���� �ؽ�ó�� �ҷ��´�. 
	LoadTexture();

	// ��Ʈ �ñ׳�ó�� 
	// ������ �� �׸��� ���̴��� �ҷ��´�.
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	
	// ������Ʈ�� ����
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildBoxGeometry();
	BuildTreeSpritesGeometry();

	// ����(���͸���)�� �ҷ��´�.
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSO();

	// �ʱ�ȭ ����� �����մϴ�.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ʱ�ȭ�� �Ϸ�� ������ ��ٸ��ϴ�.
	FlushCommandQueue();

	return true;
}

void WaveSimulator::OnResize()
{
	Core::OnResize();

	// â ũ�Ⱑ �����Ǿ����Ƿ� ��Ⱦ�� ������Ʈ�ϰ� ���� ����� �ٽ� ����մϴ�
	// XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	// XMStoreFloat4x4(&m_Proj, P);
	GET_SINGLE(Camera)->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

inline void WaveSimulator::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size(), 
			(UINT)m_Materials.size(), m_Waves->VertexCount()));
	}
}

inline void WaveSimulator::LoadTexture()
{
	GET_SINGLE(ResourceManager)->LoadTexture("grassTex", L"Graphics/Texture/Datas/grass.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("waterTex", L"Graphics/Texture/Datas/water1.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("fenceTex", L"Graphics/Texture/Datas/WireFence.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("treeArrayTex", L"Graphics/Texture/Datas/treeArray2.dds");
}

// ������ ���� �����Ѵ�.
// �̴� �ڿ��� ������ ���������ο� ���� �� ����� ���Դϴ�.
// Input : void
// Output : void
inline void WaveSimulator::BuildDescriptorHeaps()
{
	//
	// SRV ���� �����Ѵ�.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)));

	//
	// ���� �� ���� �����.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = GET_SINGLE(ResourceManager)->FindTexture("grassTex")->pResource;
	auto waterTex = GET_SINGLE(ResourceManager)->FindTexture("waterTex")->pResource;
	auto fenceTex = GET_SINGLE(ResourceManager)->FindTexture("fenceTex")->pResource;
	auto treeArrayTex = GET_SINGLE(ResourceManager)->FindTexture("treeArrayTex")->pResource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	m_d3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// ���� �����ڸ� �����´�.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// ���� �����ڸ� �����´�.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// ���� �����ڸ� �����´�.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	// �� �κ��� Texture2D �迭�̱� ������
	// �ٸ� ������� ������ ��������� �Ѵ�.
	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;	// ���� �Ӹ� �� ������
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	m_d3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);
}

// ��Ʈ ����� ������ ���̺��� �����մϴ�.
// 
// Input : void
// Output : void
inline void WaveSimulator::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// ��Ʈ �Ű������� ���̺�, ��Ʈ ������ �Ǵ� ��Ʈ ����� �� �ֽ��ϴ�.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// ���� ��: ���� ���� �߻��ϴ� �Ϳ��� ���� ���� �߻��ϴ� ������ �����Ͻʽÿ�.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// ��Ʈ ������ ��Ʈ �Ű������� �迭�Դϴ�.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// ���� ��� ���۷� ������ ������ ������ ����Ű�� ���� �������� ��Ʈ ������ ����ϴ�.
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

// ���̴��� ����Ʈ�ڵ�� �������ϰ�
// �Է� ���̾ƿ��� ���� ���̴��� �ѱ��.
inline void WaveSimulator::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	// D3D_SHADER_MACRO ����ü�� �̿��ؼ� ���̴� ���������ο� �Ű������� �Ѱ��� �� �ִ�.
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

	// ���̴��� �������ؼ� ����Ʈ�ڵ�� ������.
	// �׸��� �� �ý����� GPU�� �°� ������ ����Ƽ�� ������� �������� �Ѵ�.
	GET_SINGLE(ResourceManager)->LoadShader("standardVS", L"Graphics\\Shader\\BlendDefault.hlsl", nullptr, "VS", "vs_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("opaquePS", L"Graphics\\Shader\\BlendDefault.hlsl", defines, "PS", "ps_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("alphaTestedPS", L"Graphics\\Shader\\BlendDefault.hlsl", alphaTestDefines, "PS", "ps_5_1");

	// ������Ʈ�� ���̴��� �������.
	GET_SINGLE(ResourceManager)->LoadShader("treeSpriteVS", L"Graphics\\Shader\\GeometryShader.hlsl", nullptr, "VS", "vs_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("treeSpriteGS", L"Graphics\\Shader\\GeometryShader.hlsl", nullptr, "GS", "gs_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("treeSpritePS", L"Graphics\\Shader\\GeometryShader.hlsl", alphaTestDefines, "PS", "ps_5_1");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_TreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

inline void WaveSimulator::BuildWavesGeometryBuffers()
{
	std::vector<std::uint16_t> indices(3 * m_Waves->TriangleCount()); // 3 indices per face
	assert(m_Waves->VertexCount() < 0x0000ffff);

	// �� ���帶�� �ݺ��Ѵ�.
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

	auto geo = new MeshGeometry();
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
	auto grass = GET_SINGLE(ResourceManager)->BuildMaterial("grass", 0, 0,
		XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.125f);
	auto water = GET_SINGLE(ResourceManager)->BuildMaterial("water", 1, 1,
		XMFLOAT4(0.0f, 0.2f, 0.6f, 0.5f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.0);
	auto wirefence = GET_SINGLE(ResourceManager)->BuildMaterial("wirefence", 2, 2,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.25f);
	auto treeSprites = GET_SINGLE(ResourceManager)->BuildMaterial("treeSprites", 3, 3,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.125f);

	m_Materials["grass"] = std::move(grass);
	m_Materials["water"] = std::move(water);
	m_Materials["wirefence"] = std::move(wirefence);
	m_Materials["treeSprites"] = std::move(treeSprites);
}

inline void WaveSimulator::BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// ������ ��Ҹ� �����ϰ� �� �������� ���� �Լ��� �����մϴ�.
	// ���� ���̿� ���� ������ ������ �����Ͽ� ��ó�� ���̴� �غ�,
	// Ǯ�� ������ ���� ��� �� �� ���� ����츮�� ����ϴ�. 
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

	auto geo = new MeshGeometry();
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

	auto geo = new MeshGeometry();
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

inline void WaveSimulator::BuildTreeSpritesGeometry()
{
	static const int treeCount = 16;
	std::array<TreeSpriteVertex, 16> vertices;
	for (UINT i = 0; i < treeCount; ++i)
	{
		float x = MathHelper::RandF(-45.0f, 45.0f);
		float z = MathHelper::RandF(-45.0f, 45.0f);
		float y = GetHillsHeight(x, z);

		// Move tree slightly above land height.
		y += 8.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}

	std::array<std::uint16_t, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	m_Geometries["treeSpritesGeo"] = std::move(geo);
}

inline void WaveSimulator::BuildRenderItems()
{
	auto wavesRitem = std::make_unique<MeshObject>();
	wavesRitem->SetWorldMatrix(MathHelper::Identity4x4());
	wavesRitem->SetTexTransform(XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->SetObjCBIdx(0);
	wavesRitem->SetMaterial(m_Materials["water"]);
	wavesRitem->SetGeometry(m_Geometries["waterGeo"]);
	wavesRitem->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	wavesRitem->SetIdxCnt(wavesRitem->GetGeometry()->DrawArgs["grid"].IndexCount);
	wavesRitem->SetStartIdxLocation(wavesRitem->GetGeometry()->DrawArgs["grid"].StartIndexLocation);
	wavesRitem->SetBaseVertexLocation(wavesRitem->GetGeometry()->DrawArgs["grid"].BaseVertexLocation);

	m_WaveRenderItem = wavesRitem.get();
	m_RenderitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<MeshObject>();
	gridRitem->SetWorldMatrix(MathHelper::Identity4x4());
	gridRitem->SetTexTransform(XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->SetObjCBIdx(1);
	gridRitem->SetMaterial(m_Materials["grass"]);
	gridRitem->SetGeometry(m_Geometries["landGeo"]);
	gridRitem->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gridRitem->SetIdxCnt(gridRitem->GetGeometry()->DrawArgs["grid"].IndexCount);
	gridRitem->SetStartIdxLocation(gridRitem->GetGeometry()->DrawArgs["grid"].StartIndexLocation);
	gridRitem->SetBaseVertexLocation(gridRitem->GetGeometry()->DrawArgs["grid"].BaseVertexLocation);

	m_RenderitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	auto boxRitem = std::make_unique<MeshObject>();
	boxRitem->SetWorldMatrix(MathHelper::Identity4x4());
	boxRitem->SetTexTransform(XMMatrixScaling(3.0f, 2.0f, -9.0f));
	boxRitem->SetObjCBIdx(2);
	boxRitem->SetMaterial(m_Materials["wirefence"]);
	boxRitem->SetGeometry(m_Geometries["boxGeo"]);
	boxRitem->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	boxRitem->SetIdxCnt(boxRitem->GetGeometry()->DrawArgs["box"].IndexCount);
	boxRitem->SetStartIdxLocation(boxRitem->GetGeometry()->DrawArgs["box"].StartIndexLocation);
	boxRitem->SetBaseVertexLocation(boxRitem->GetGeometry()->DrawArgs["box"].BaseVertexLocation);

	m_RenderitemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem.get());

	auto treeSpritesRitem = std::make_unique<MeshObject>();
	treeSpritesRitem->SetWorldMatrix(MathHelper::Identity4x4());
	treeSpritesRitem->SetObjCBIdx(3);
	treeSpritesRitem->SetMaterial(m_Materials["treeSprites"]);
	treeSpritesRitem->SetGeometry(m_Geometries["treeSpritesGeo"]);
	treeSpritesRitem->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	treeSpritesRitem->SetIdxCnt(treeSpritesRitem->GetGeometry()->DrawArgs["points"].IndexCount);
	treeSpritesRitem->SetStartIdxLocation(treeSpritesRitem->GetGeometry()->DrawArgs["points"].StartIndexLocation);
	treeSpritesRitem->SetBaseVertexLocation(treeSpritesRitem->GetGeometry()->DrawArgs["points"].BaseVertexLocation);

	m_RenderitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());

	m_allRenderItems.push_back(std::move(wavesRitem));
	m_allRenderItems.push_back(std::move(gridRitem));
	m_allRenderItems.push_back(std::move(boxRitem));
	m_allRenderItems.push_back(std::move(treeSpritesRitem));
}

// ���������� ���� ��ü(Pipeline State Object)�� �����Ѵ�.
// ���⼭�� ���ݱ��� �Է� ���̾ƿ�, ����/�ȼ� ���̴��� ����� �����Ͷ����� ���¸� ������ ����
// ������ ���������ο� ��� �� ���¸� ������ �� �ִ� PSO�� �����մϴ�.
inline void WaveSimulator::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// ������ü(Opaque) ������ ������
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("standardVS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("standardVS")->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("opaquePS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("opaquePS")->GetBufferSize()
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

	// ������ ���� �ڵ�
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;		// �� ����
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPSODesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(
		&transparentPSODesc, IID_PPV_ARGS(&m_PSOs["transparent"])));

	// ���� ������ ���� PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestPSODesc = opaquePsoDesc;
	alphaTestPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("alphaTestedPS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("alphaTestedPS")->GetBufferSize()
	};
	alphaTestPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(
		&alphaTestPSODesc, IID_PPV_ARGS(&m_PSOs["alphaTested"])));


	// 
	// Ʈ���� ���� PSO �ڵ��Դϴ�.
	// 
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("treeSpriteVS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("treeSpriteVS")->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("treeSpriteGS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("treeSpriteGS")->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("treeSpritePS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("treeSpritePS")->GetBufferSize()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { m_TreeSpriteInputLayout.data(), (UINT)m_TreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, 
		IID_PPV_ARGS(&m_PSOs["treeSprites"])));


	//
	// �������� ���̾������� ��ü�� ���� PSO�Դϴ�.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentWireframePsoDesc = transparentPSODesc;
	transparentWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&transparentWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["transparent_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaframePsoDesc = alphaTestPSODesc;
	alphaframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&alphaframePsoDesc, IID_PPV_ARGS(&m_PSOs["alpha_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpriteWireframePsoDesc = treeSpritePsoDesc;
	treeSpriteWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&treeSpriteWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["treeSprite_wireframe"])));
}

inline void WaveSimulator::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<MeshObject*>& ritems, 
	float fDeltaTime)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_curFrameResource->ObjectCB->Resource();
	auto matCB = m_curFrameResource->MaterialCB->Resource();

	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		ri->Render(cmdList, fDeltaTime);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->GetMaterial()->nDiffuseSrvHeapIdx, m_CbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +
			ri->GetObjCBIdx() * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + 
			ri->GetMaterial()->nMatCBIdx * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->GetIdxCnt(), 1, ri->GetStartIdxLocation(), ri->GetBaseVertexLocation(), 0);
	}
}

inline std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> WaveSimulator::GetStaticSamplers()
{
	// ���ø����̼ǿ��� �Ϲ������� �Ҽ��� ���÷��� �ʿ��մϴ�. 
	// ���� �װ͵��� ��� �̸� �����ϰ� ��Ʈ ������ �Ϻη� ����� �� �ֵ��� �����ϴ� ���� �����ϴ�.

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
	if (GET_SINGLE(InputManager)->KeyDown("MouseLButton"))
	{
		// �� �ȼ��� 4���� 1���� �ش��ϵ��� �մϴ�.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		GET_SINGLE(Camera)->Pitch(dy);
		GET_SINGLE(Camera)->RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

inline void WaveSimulator::OnMouseUp(int x, int y)
{
}

void WaveSimulator::Input(float fDeltaTime)
{
	if (GET_SINGLE(InputManager)->KeyDown("Escape"))
	{
		m_bLoop = false;
		PostQuitMessage(0);
	}

	if (GET_SINGLE(InputManager)->KeyDown("WireFrame"))
	{
		cout << "1 pressed" << endl;
		m_IsWireframe = true;
	}

	if (GET_SINGLE(InputManager)->KeyDown("MoveFront"))
	{
		GET_SINGLE(Camera)->Walk(20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyDown("MoveBack"))
	{
		GET_SINGLE(Camera)->Walk(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyDown("MoveLeft"))
	{
		GET_SINGLE(Camera)->Strafe(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyDown("MoveRight"))
	{
		GET_SINGLE(Camera)->Strafe(20.0f * fDeltaTime);
	}

	// if (GET_SINGLE(Input)->IsKeyDown(VK_LEFT))
	// {
	// 	m_fSunTheta -= 1.0f * fDeltaTime;
	// }
	// if (GET_SINGLE(Input)->IsKeyDown(VK_RIGHT))
	// {
	// 	m_fSunTheta += 1.0f * fDeltaTime;
	// }
	// if (GET_SINGLE(Input)->IsKeyDown(VK_UP))
	// {
	// 	m_fSunPhi -= 1.0f * fDeltaTime;
	// }
	// if (GET_SINGLE(Input)->IsKeyDown(VK_DOWN))
	// {
	// 	m_fSunPhi += 1.0f * fDeltaTime;
	// }
}

inline void WaveSimulator::AnimateMaterials(float fDeltaTime)
{
	// Scroll the water material texture coordinates.
	auto waterMat = m_Materials["water"];

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

	// ������ ����Ǿ����Ƿ� cbuffer�� ������Ʈ�ؾ� �մϴ�.
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

// ������Ʈ�� ��� ���۸� �����Ѵ�.
inline void WaveSimulator::UpdateObjectCBs(float fDeltaTime)
{
	auto curObjectCB = m_curFrameResource->ObjectCB.get();

	// ������� �ٲ���� ������ cbuffer �ڷḦ �����ؾ� �Ѵ�.
	// �̷��� ������ ������ �ڿ����� �����ؾ� �Ѵ�.
	for (auto& e : m_allRenderItems)
	{
		int dirty = e->GetFrameDirty();
		if (dirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->GetWorldMatrix());
			XMMATRIX texTransform = XMLoadFloat4x4(&e->GetTexTransform());

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			curObjectCB->CopyData(e->GetObjCBIdx(), objConstants);

			// ���� ������ �ڿ����� �Ѿ��.
			e->SetFrameDirty(dirty--);
		}
	}
}

inline void WaveSimulator::UpdateMaterialCBs(float fDeltaTime)
{
	auto curMaterialCB = m_curFrameResource->MaterialCB.get();

	for (auto& e : m_Materials)
	{
		// ������� ������ ���� cbuffer�� �����Ѵ�.
		// �̷��� ������ ������ �ڿ����� �����ؾ� �Ѵ�.
		Material* mat = e.second;

		if (mat->nFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConst.FresnelR0 = mat->FresnelR0;
			matConst.fRoughness = mat->fRoughness;
			XMStoreFloat4x4(&matConst.MatTransform, XMMatrixTranspose(matTransform));

			curMaterialCB->CopyData(mat->nMatCBIdx, matConst);

			// ���� ������ �ڿ����� �Ѿ��.
			mat->nFramesDirty--;
		}
	}
}

inline void WaveSimulator::UpdateMainPassCB(float fDeltaTime)
{
	XMMATRIX view = GET_SINGLE(Camera)->GetView();
	XMMATRIX proj = GET_SINGLE(Camera)->GetProj();

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
	m_tMainPassCB.EyePosW = GET_SINGLE(Camera)->GetPosition3f();

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
	// �� 1/4�ʸ��� ������ ���̺긦 �����մϴ�.
	static float t_base = 0.0f;
	if ((GET_SINGLE(Timer)->GetTotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, m_Waves->RowCount() - 5);
		int j = MathHelper::Rand(4, m_Waves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		m_Waves->Disturb(i, j, r);
	}

	// ���̺� �ùķ��̼��� ������Ʈ�մϴ�.
	m_Waves->Update(fDeltaTime);

	// �� �ַ������ ���̺� ���� ���۸� ������Ʈ�Ѵ�.
	auto currWavesVB = m_curFrameResource->WavesVB.get();
	for (int i = 0; i < m_Waves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = m_Waves->Position(i);
		v.Normal = m_Waves->Normal(i);

		// [-w/2,w/2] --> [0,1] ������ ���� ��ġ���� tex-coords �Ļ�
		v.TexC.x = 0.5f + v.Pos.x / m_Waves->Width();
		v.TexC.y = 0.5f - v.Pos.z / m_Waves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// �ĵ� ���� �������� ���� VB�� ���� ������ VB�� �����մϴ�.
	m_WaveRenderItem->GetGeometry()->VertexBufferGPU = currWavesVB->Resource();
}

int WaveSimulator::Update(float fDeltaTime)
{
	GET_SINGLE(Camera)->UpdateViewMatrix();

	m_fSunPhi = MathHelper::Clamp(m_fSunPhi, 1.0f, XM_PIDIV2);

	// ���� ������ ���ҽ� �迭�� ��ȯ�Ѵ�.
	m_nCurFrameResourceIdx = (m_nCurFrameResourceIdx + 1) % gNumFrameResource;
	m_curFrameResource = m_frameResources[m_nCurFrameResourceIdx].get();

	// GPU�� ���� ������ ���ҽ��� ��� ó���� �Ϸ��ߴ°��� �Ǻ��Ѵ�
	// ���� ���� ��� GPU�� �� ��Ÿ�� �������� ����� �Ϸ��� ������ ��ٸ���.
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
	GET_SINGLE(CollisionManager)->Collision(fDeltaTime);
}

void WaveSimulator::Render(float fDeltaTime)
{
	// ���� Ŀ�ǵ� ����Ʈ �Ҵ��ڸ� �����´�.
	auto cmdListAlloc = m_curFrameResource->pCmdListAlloc;

	// Ŀ�ǵ� ��ϰ� ���õ� �޸𸮸� �����մϴ�.
	// ����� Ŀ�ǵ� ����Ʈ�� GPU���� ������ �Ϸ��� ��쿡�� �缳���� �� �ֽ��ϴ�.
	ThrowIfFailed(cmdListAlloc->Reset());

	// Ŀ�ǵ� ����Ʈ�� ExecuteCommandList�� ���� ��� ��⿭�� �߰��� �� �缳���� �� �ֽ��ϴ�.
	// Ŀ�ǵ� ����Ʈ�� �����ϸ� �޸𸮰� ����˴ϴ�.
	// �⺻������ ���̾� �������� ���� �ֽ��ϴ�.
	if (m_IsWireframe)
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque_wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"].Get()));
	}

	// ����Ʈ�� Scissor Rect�� �����մϴ�. �̰��� Ŀ�ǵ� ����Ʈ�� �缳���� ������ �缳���Ǿ�� �մϴ�.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// ���ҽ� ��뷮�� ���� ���� ��ȯ�� ��Ÿ���ϴ�.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// �� ���ۿ� ���� ���۸� ����ϴ�.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&m_tMainPassCB.FogColor, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// �������� ���۸� �����մϴ�.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// ������ ���� �����ͼ� �̸� �����Ѵ�.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// �׷��Ƚ� ��Ʈ ������ �����մϴ�.
	// SetGraphicsRootSignature�� �̿��ϸ� ������ ���̺��� �����ͼ� ���������ο� ���� �� �ֽ��ϴ�.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// �н��� ��� ���۸� ���ε��մϴ�. �� �۾��� �н��� �� ���� �����ϸ� �˴ϴ�.
	auto passCB = m_curFrameResource->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Opaque]);

	if (m_IsWireframe)
	{
		m_CommandList->SetPipelineState(m_PSOs["alpha_wireframe"].Get());
	}
	else
	{
		m_CommandList->SetPipelineState(m_PSOs["alphaTested"].Get());
	}
	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::AlphaTested]);

	if (m_IsWireframe)
	{
		m_CommandList->SetPipelineState(m_PSOs["treeSprite_wireframe"].Get());
	}
	else
	{
		m_CommandList->SetPipelineState(m_PSOs["treeSprites"].Get());
	}
	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	if (m_IsWireframe)
	{
		m_CommandList->SetPipelineState(m_PSOs["transparent_wireframe"].Get());
	}
	else
	{
		m_CommandList->SetPipelineState(m_PSOs["transparent"].Get());
	}
	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Transparent]);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// ���ڵ� ��� �Ϸ�
	ThrowIfFailed(m_CommandList->Close());

	// ������ ��⿭�� ��� ����� �߰��մϴ�.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ĸ� �� ���� ���� ��ü
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// �� �潺 ����Ʈ���� ����� ǥ���Ϸ��� �潺 ���� ������Ų��.
	m_curFrameResource->nFence = ++m_CurrentFence;

	// ��� ��⿭�� ����� �߰��Ͽ� �� �潺 ����Ʈ�� �����Ѵ�.
	// GPU Ÿ�Ӷ��ο� �����Ƿ� GPU�� �� Signal() ������ ��� ��� ó���� �Ϸ��� ������ �� �潺 ����Ʈ�� �������� �ʴ´�.
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}