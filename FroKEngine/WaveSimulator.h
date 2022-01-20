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

	// ���̴����� ����� �����ؼ� �����Ѵ�.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// ������ ���ҽ��� �����ϱ� ���� vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	UINT m_CbvSrvDescriptorSize = 0;

	// ��� ���� �������� ����Ʈ.
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;

	// �ĵ��� ���� ������
	std::unique_ptr<Waves> m_Waves;
	RenderItem* m_WaveRenderItem;

	// ���׸����� �����ϱ� ���� ��
	std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;

	// PSO ���¿� ���� �޶����� ������ �����۵��̴�.
	std::vector<RenderItem*> m_RenderitemLayer[(int)RenderLayer::Count];

	bool m_IsWireframe = false;

	// �н� ������� �����ص� ����ü
	PassConstants m_tMainPassCB;

	// ������Ʈ�� ������ �����ϱ� ���� map
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

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

	// ī�޶� Ŭ����
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

	// ���� Ŀ�ǵ� ����Ʈ�� �ʱ�ȭ �Ѵ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, so we have
	// to query this information.
	m_CbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// �ĵ��� ���� ������ �Ѵ�.
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

// ������ ���� �����Ѵ�.
// �̴� �ڿ��� ������ ���������ο� ���� �� ����� ���Դϴ�.
// Input : void
// Output : void
inline void WaveSimulator::BuildDescriptorHeaps()
{
	UINT objCnt = (UINT)m_OpaqueRenderItems.size();

	// �� ������ �ڿ��� ��ü���� �ϳ��� CBV �����ڰ� �ʿ��ϴ�.
	// +1�� �� ������ �ڿ��� �ʿ��� �н��� CBV�� ���� ���̴�.
	UINT nDescriptors = (objCnt + 1) * gNumFrameResource;

	// �н��� CBV�� ���� �������� �����صд�.
	// �̵��� ������ �� �������̴�.
	m_passCbvOffset = objCnt * gNumFrameResource;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = nDescriptors;
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
inline void WaveSimulator::BuildConstantBufferViews()
{
	// 256����Ʈ ����� ����ϴ�.
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	UINT objCnt = (UINT)m_OpaqueRenderItems.size();

	// �� ������ �ڿ��� ��ü���� �ϳ��� CBV �����ڰ� �ʿ��ϴ�.
	for (int frameIdx = 0; frameIdx < gNumFrameResource; ++frameIdx)
	{
		auto objCB = m_frameResources[frameIdx]->ObjectCB->Resource();

		for (UINT i = 0; i < objCnt; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objCB->GetGPUVirtualAddress();

			// ���� ���ۿ��� i��° ��ü�� ��� ������ ������
			cbAddress += i * objCBByteSize;

			// ������ �� �ȿ��� cbv ������Ʈ �������� ���Ѵ�.
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

	// ������ 3���� �����ڴ� �� ������ ���ҽ��� ���� �н� ��� ���� �䰡 �ȴ�.
	for (int frameIndex = 0; frameIndex < gNumFrameResource; ++frameIndex)
	{
		auto passCB = m_frameResources[frameIndex]->PassCB->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// ������ �� ���� �н� ��� ���� �並 ���� �������� ����Ѵ�.
		int heapIndex = m_passCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

// ��Ʈ ����� ������ ���̺��� �����մϴ�.
// 
// Input : void
// Output : void
inline void WaveSimulator::BuildRootSignature()
{
	// ��Ʈ �Ű������� ���̺�, ��Ʈ ������ �Ǵ� ��Ʈ ����� �� �ֽ��ϴ�.
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Create root CBV.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);

	// ��Ʈ ������ ��Ʈ �Ű������� �迭�Դϴ�.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	// ���̴��� �������ؼ� ����Ʈ�ڵ�� ������.
	// �׸��� �� �ý����� GPU�� �°� ������ ����Ƽ�� ������� �������� �Ѵ�.
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
	// �������� ���̾������� ��ü�� ���� PSO�Դϴ�.
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
		// �� �ȼ��� 4���� 1���� �ش��ϵ��� �մϴ�.
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

// ������Ʈ�� ��� ���۸� �����Ѵ�.
inline void WaveSimulator::UpdateObjectCBs(float fDeltaTime)
{
	auto curObjectCB = m_curFrameResource->ObjectCB.get();

	// ������� �ٲ���� ������ cbuffer �ڷḦ �����ؾ� �Ѵ�.
	// �̷��� ������ ������ �ڿ����� �����ؾ� �Ѵ�.
	for (auto& e : m_allRenderItems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		curObjectCB->CopyData(e->objCBIdx, objConstants);

		// ���� ������ �ڿ����� �Ѿ��.
		e->nFramesDirty--;
	}
}

inline void WaveSimulator::UpdateMaterialCBs(float fDeltaTime)
{
	auto curMaterialCB = m_curFrameResource->MaterialCB.get();

	for (auto& e : m_Materials)
	{
		// ������� ������ ���� cbuffer�� �����Ѵ�.
		// �̷��� ������ ������ �ڿ����� �����ؾ� �Ѵ�.
		Material* mat = e.second.get();

		if (mat->nFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConst;
			matConst.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConst.FresnelR0 = mat->FresnelR0;
			matConst.fRoughness = mat->fRoughness;

			curMaterialCB->CopyData(mat->nMatCBIdx, matConst);

			// ���� ������ �ڿ����� �Ѿ��.
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

		currWavesVB->CopyData(i, v);
	}

	// �ĵ� ���� �������� ���� VB�� ���� ������ VB�� �����մϴ�.
	m_WaveRenderItem->pGeometry->VertexBufferGPU = currWavesVB->Resource();
}

int WaveSimulator::Update(float fDeltaTime)
{
	m_Camera.UpdateViewMatrix();

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
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// �������� ���۸� �����մϴ�.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// �׷��Ƚ� ��Ʈ ������ �����մϴ�.
	// SetGraphicsRootSignature�� �̿��ϸ� ������ ���̺��� �����ͼ� ���������ο� ���� �� �ֽ��ϴ�.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// �н��� ��� ���۸� ���ε��մϴ�. �� �۾��� �н��� �� ���� �����ϸ� �˴ϴ�.
	auto passCB = m_curFrameResource->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(m_CommandList.Get(), m_RenderitemLayer[(int)RenderLayer::Opaque]);

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

