#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/FrameResource.h"
#include "Graphics/RenderItem.h"
#include "Graphics/GeometryGenerator.h"
#include "Graphics/Material.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;


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

	// ������ ���ҽ��� �����Ѵ�.
	void BuildFrameResources();

	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildShapeGeometry();
	void BuildRenderItems();
	void BuildPSO();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	void UpdateCamera(float fDeltaTime);
	void UpdateObjectCBs(float fDeltaTime);
	void UpdateMaterialCBs(float fDeltaTime);
	void UpdateMainPassCB(float fDeltaTime);

	// void MakeRenderItem();

private :
	void OnMouseDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseUp(int x, int y);

private :
	ComPtr<ID3D12RootSignature>		m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>	m_CbvHeap = nullptr;

	// �н��� ����� ������Ʈ ������� �����ϱ� ���� ����
	std::unique_ptr<UploadBuffer<PassConstants>> m_PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;
	UINT m_passCbvOffset = 0;

	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	// ���̴����� ����� �����ؼ� �����Ѵ�.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// ������ ���ҽ��� �����ϱ� ���� vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	// ��� ���� �������� ����Ʈ.
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;
	
	// ��� ���׸��� ����
	std::unordered_map<std::string, std::unique_ptr<Material>> m_Material;

	bool m_IsWireframe = true;

	// �н� ������� �����ص� ����ü
	PassConstants m_tMainPassCB;

	// ������Ʈ�� ������ �����ϱ� ���� map
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	XMFLOAT4X4	m_World	= MathHelper::Identity4x4();
	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4	m_View	= MathHelper::Identity4x4();
	XMFLOAT4X4	m_Proj	= MathHelper::Identity4x4();

	float		m_Theta = 1.5f * XM_PI;
	// float		m_Phi = XM_PIDIV4;
	float		m_Phi = 0.2f * XM_PI;
	float		m_Radius = 15.0f;

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

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildConstantBufferViews();
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

inline void FroKEngine::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size()));
	}
}

// ������ ���� �����Ѵ�.
// �̴� �ڿ��� ������ ���������ο� ���� �� ����� ���Դϴ�.
// Input : void
// Output : void
inline void FroKEngine::BuildDescriptorHeaps()
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
inline void FroKEngine::BuildConstantBufferViews()
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
inline void FroKEngine::BuildRootSignature()
{
	// ���̴� ���α׷��� �Ϲ������� �Է����� ���ҽ��� �ʿ�� �մϴ�(��� ����, �ؽ�ó, ���÷�).
	// ��Ʈ ������ ���̴� ���α׷��� ����ϴ� ���ҽ��� �����մϴ�.
	// ���̴� ���α׷��� �Լ���, �Է� ���ҽ��� �Լ� �Ű������� �����ϸ�
	// ��Ʈ ������ �Լ� ������ �����ϴ� ������ ������ �� �ֽ��ϴ�.

	// ��Ʈ �Ű������� ���̺�, ��Ʈ ������ �Ǵ� ��Ʈ ����� �� �ֽ��ϴ�.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	// CBV�� ���� ������ ���̺��� ����ϴ�.
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// ���̺��� ������ ����
		0);	// �� ��Ʈ �Ű������� ���� ���̴� �μ����� ���� �������� ��ȣ( register (b0) )

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// ���̺��� ������ ����
		1);	// �� ��Ʈ �Ű������� ���� ���̴� �μ����� ���� �������� ��ȣ( register (b1) )

	slotRootParameter[0].InitAsDescriptorTable(1,	// ����(range) ����
		&cbvTable0);	// �������� �迭�� ����Ű�� ������
	slotRootParameter[1].InitAsDescriptorTable(1,	// ����(range) ����
		&cbvTable1);	// �������� �迭�� ����Ű�� ������

	// ��Ʈ ������ ��Ʈ �Ű������� �迭�Դϴ�.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
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
	m_shaders["standardVS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "PS", "ps_5_1");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

// �ڽ� ������Ʈ���� �����Ѵ�.
inline void FroKEngine::BuildBoxGeometry()
{
	// ���� ���� �� �� ������ �����.
}

inline void FroKEngine::BuildShapeGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData box = geoGenerator.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGenerator.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGenerator.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGenerator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	// �� �ڵ�� ��� ���� ������ �ϳ��� Ŀ�ٶ� ����/�ε��� ���ۿ� ��´�.
	// ���� ���ۿ��� �� �κ� �޽ð� �����ϴ� �������� ������ �ʿ䰡 �ִ�.

	// ����� ���� ���ۿ����� �� ��ü�� ���� �ε����� ������ �����鿡 �����մϴ�.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// ����� �ε��� ���ۿ����� �� ��ü�� ���� �ε����� ������ �����鿡 �����صӴϴ�.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// ���� / �ε��� ���ۿ��� �� ��ü�� �����ϴ� ������ ��Ÿ���� SubmeshGeometry ��ü�� �����Ѵ�.
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	// �ʿ��� ���� ���е��� �����ϰ�, ��� �޽��� �������� �ϳ��� ���� ���۷� �ִ´�.

	auto totalVertexCount = box.Vertices.size() + grid.Vertices.size() +
		sphere.Vertices.size() + cylinder.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGreen);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::Crimson);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::SteelBlue);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(),
		std::begin(box.GetIndices16()),
		std::end(box.GetIndices16()));
	indices.insert(indices.end(),
		std::begin(grid.GetIndices16()),
		std::end(grid.GetIndices16()));
	indices.insert(indices.end(),
		std::begin(sphere.GetIndices16()),
		std::end(sphere.GetIndices16()));
	indices.insert(indices.end(),
		std::begin(cylinder.GetIndices16()),
		std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	// GPU���� ����� ���۸� ���д�.
	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	m_Geometries[geo->Name] = std::move(geo);
}

inline void FroKEngine::BuildRenderItems()
{
	auto boxRenderItem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRenderItem->World,
		XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	boxRenderItem->objCBIdx = 0;
	boxRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
	boxRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRenderItem->nIdxCnt = boxRenderItem->pGeometry->DrawArgs["box"].IndexCount;
	boxRenderItem->nStartIdxLocation = boxRenderItem->pGeometry->DrawArgs["box"].StartIndexLocation;
	boxRenderItem->nBaseVertexLocation = boxRenderItem->pGeometry->DrawArgs["box"].BaseVertexLocation;
	m_allRenderItems.push_back(std::move(boxRenderItem));

	auto gridRenderItem = std::make_unique<RenderItem>();
	gridRenderItem->World = MathHelper::Identity4x4();
	gridRenderItem->objCBIdx = 1;
	gridRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
	gridRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRenderItem->nIdxCnt = gridRenderItem->pGeometry->DrawArgs["grid"].IndexCount;
	gridRenderItem->nStartIdxLocation = gridRenderItem->pGeometry->DrawArgs["grid"].StartIndexLocation;
	gridRenderItem->nBaseVertexLocation = gridRenderItem->pGeometry->DrawArgs["grid"].BaseVertexLocation;
	m_allRenderItems.push_back(std::move(gridRenderItem));

	// ��յ�� ���� �� �ٷ� ��ġ�Ѵ�.
	UINT objCBIdx = 2;
	for (int i = 0; i < 5; i++)
	{
		auto leftCylRenderItem = std::make_unique<RenderItem>();
		auto rightCylRenderItem = std::make_unique<RenderItem>();
		auto leftSphereRenderItem = std::make_unique<RenderItem>();
		auto rightSphereRenderItem = std::make_unique<RenderItem>();

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

		XMStoreFloat4x4(&leftCylRenderItem->World, leftCylWorld);
		leftCylRenderItem->objCBIdx = objCBIdx++;
		leftCylRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		leftCylRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRenderItem->nIdxCnt = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].IndexCount;
		leftCylRenderItem->nStartIdxLocation = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRenderItem->nBaseVertexLocation = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(leftCylRenderItem));

		XMStoreFloat4x4(&rightCylRenderItem->World, rightCylWorld);
		rightCylRenderItem->objCBIdx = objCBIdx++;
		rightCylRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		rightCylRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRenderItem->nIdxCnt = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].IndexCount;
		rightCylRenderItem->nStartIdxLocation = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRenderItem->nBaseVertexLocation = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(rightCylRenderItem));

		XMStoreFloat4x4(&leftSphereRenderItem->World, leftSphereWorld);
		leftSphereRenderItem->objCBIdx = objCBIdx++;
		leftSphereRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		leftSphereRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRenderItem->nIdxCnt = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].IndexCount;
		leftSphereRenderItem->nStartIdxLocation = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRenderItem->nBaseVertexLocation = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(leftSphereRenderItem));

		XMStoreFloat4x4(&rightSphereRenderItem->World, rightSphereWorld);
		rightSphereRenderItem->objCBIdx = objCBIdx++;
		rightSphereRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		rightSphereRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRenderItem->nIdxCnt = rightSphereRenderItem->pGeometry->DrawArgs["sphere"].IndexCount;
		rightSphereRenderItem->nStartIdxLocation = rightSphereRenderItem->pGeometry->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRenderItem->nBaseVertexLocation = rightSphereRenderItem->pGeometry->DrawArgs["sphere"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(rightSphereRenderItem));
	}

	for (auto& e : m_allRenderItems)
	{
		m_OpaqueRenderItems.push_back(e.get());
	}
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
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};

	// �����Ͷ����� �κ��� ���̴��� ���� ���α׷����� �� ����
	// �ܼ��� ������ ������ �κ��̴�.
	// �����ͳ� �� �� �������� �����ݴϴ�.
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX; // ����ǥ��ȭ�� �����մϴ�.(���⼭�� �� � ǥ���� ��Ȱ��ȭ���� �ʴ� MAX�� �ֽ��ϴ�.)
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// �⺻ ������ �ﰢ���Դϴ�
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_BackBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_DepthStencilFormat;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = psoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque_wireframe"])));
}

inline void FroKEngine::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_curFrameResource->ObjectCB->Resource();
	auto matCB = m_curFrameResource->MaterialCB->Resource();

	// �� �������� �����۸��� ���������ο� ���´�
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->pGeometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->pGeometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->primitiveType);

		// �� ��ü �� �� ������ ���ҽ��� ���� ������ ���� CBV�� ���� ������.
		// UINT cbvIndex = m_nCurFrameResourceIdx * (UINT)m_OpaqueRenderItems.size() + ri->objCBIdx;
		// auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
		// cbvHandle.Offset(cbvIndex, m_CbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress()
			+ ri->objCBIdx * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress()
			+ ri->Mat->MatCBIdx * objCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

		cmdList->DrawIndexedInstanced(ri->nIdxCnt, 1, ri->nStartIdxLocation, ri->nBaseVertexLocation, 0);
	}
}

inline void FroKEngine::OnMouseDown(int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
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
		m_Radius = MathHelper::Clamp(m_Radius, 5.0f, 150.0f);
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

	if (GET_SINGLE(Input)->IsKeyDown('1'))
	{
		m_IsWireframe != m_IsWireframe;
	}
}

inline void FroKEngine::UpdateCamera(float fDeltaTime)
{
	// Convert Spherical to Cartesian coordinates.
	m_EyePos.x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	m_EyePos.z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	m_EyePos.y = m_Radius * cosf(m_Phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_EyePos.x, m_EyePos.y, m_EyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);
}

// ������Ʈ�� ��� ���۸� �����Ѵ�.
inline void FroKEngine::UpdateObjectCBs(float fDeltaTime)
{
	auto curObjectCB = m_curFrameResource->ObjectCB.get();

	// ������� �ٲ���� ������ cbuffer �ڷḦ �����ؾ� �Ѵ�.
	// �̷��� ������ ������ �ڿ����� �����ؾ� �Ѵ�.
	for (auto & e : m_allRenderItems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		curObjectCB->CopyData(e->objCBIdx, objConstants);

		// ���� ������ �ڿ����� �Ѿ��.
		e->nFramesDirty--;
	}
}

inline void FroKEngine::UpdateMaterialCBs(float fDeltaTime)
{
	auto curMaterialCB = m_curFrameResource->MaterialCB.get();

	for (auto& e : m_Material)
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

inline void FroKEngine::UpdateMainPassCB(float fDeltaTime)
{
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);

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
	m_tMainPassCB.EyePosW = m_EyePos;

	m_tMainPassCB.RenderTargetSize = XMFLOAT2((float)m_tRS.nWidth, (float)m_tRS.nHeight);
	m_tMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_tRS.nWidth, 1.0f / m_tRS.nHeight);

	m_tMainPassCB.NearZ = 1.0f;
	m_tMainPassCB.FarZ = 1000.0f;
	m_tMainPassCB.TotalTime = GET_SINGLE(Timer)->GetTotalTime();
	m_tMainPassCB.DeltaTime = fDeltaTime;

	auto curPassCB = m_curFrameResource->PassCB.get();
	curPassCB->CopyData(0, m_tMainPassCB);
}

int FroKEngine::Update(float fDeltaTime)
{
	UpdateCamera(fDeltaTime);

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
	UpdateMainPassCB(fDeltaTime);

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

	// ��� ���� �� ������ ���� �����ɴϴ�.
	// �̴� ������ ���������ο� �ڿ��� ���� �� ����մϴ�.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// �׷��Ƚ� ��Ʈ ������ �����մϴ�.
	// SetGraphicsRootSignature�� �̿��ϸ� ������ ���̺��� �����ͼ� ���������ο� ���� �� �ֽ��ϴ�.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	int passCbvIndex = m_passCbvOffset + m_nCurFrameResourceIdx;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, m_CbvSrvUavDescriptorSize);
	m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	DrawRenderItems(m_CommandList.Get(), m_OpaqueRenderItems);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_curFrameResource->nFence = ++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

	// ������ ����� �Ϸ�� ������ ��ٸ��ϴ�. 
	// �� ���� ��ȿ�����̸� �ܼ����� ���ؼ� �ִ� �ڵ��Դϴ�. ���߿� ������ �ڵ带 �����ϴ� ����� �����ݴϴ�.
	// ���� �����Ӵ� ��ٸ� �ʿ䰡 �����ϴ�.
	// FlushCommandQueue();
}

