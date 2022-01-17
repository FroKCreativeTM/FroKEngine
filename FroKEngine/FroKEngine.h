#pragma once

#include "Core.h"
#include "resource.h"
#include "Graphics/UploadBuffer.h"
#include "Graphics/FrameResource.h"
#include "Graphics/RenderItem.h"
#include "Graphics/GeometryGenerator.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// ������ �ڿ� 3���� ��� ���͸� �����ϱ� ���� ����
// �̷��� �ϸ� n~n+2������ �������� �̸� ������ �� �ִ�.
static const int gNumFrameResource = 3;

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

	void UpdateObjectDBs(float fDeltaTime);
	void UpdateMainPassCB(float fDeltaTime);

	void MakeRenderItem();

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

	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// ������ ���ҽ��� �����ϱ� ���� vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	// ��� ���� �������� ����Ʈ.
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;

	// �н� ������� �����ص� ����ü
	PassConstants m_tMainPassCB;

	// ������Ʈ�� ������ �����ϱ� ���� map
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	ComPtr<ID3D12PipelineState> m_PSO = nullptr;

	XMFLOAT4X4	m_World	= MathHelper::Identity4x4();
	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
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
	BuildConstantBufferViews();
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
			
		}
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

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->VertexBufferCPU));
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

		XMStoreFloat4x4(&leftCylRenderItem->World, rightCylWorld);
		leftCylRenderItem->objCBIdx = objCBIdx++;
		leftCylRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		leftCylRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRenderItem->nIdxCnt = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].IndexCount;
		leftCylRenderItem->nStartIdxLocation = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRenderItem->nBaseVertexLocation = leftCylRenderItem->pGeometry->DrawArgs["cylinder"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(leftCylRenderItem));

		XMStoreFloat4x4(&rightCylRenderItem->World, leftCylWorld);
		rightCylRenderItem->objCBIdx = objCBIdx++;
		rightCylRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		rightCylRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRenderItem->nIdxCnt = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].IndexCount;
		rightCylRenderItem->nStartIdxLocation = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRenderItem->nBaseVertexLocation = rightCylRenderItem->pGeometry->DrawArgs["cylinder"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(rightCylRenderItem));

		XMStoreFloat4x4(&leftSphereRenderItem->World, rightSphereWorld);
		leftSphereRenderItem->objCBIdx = objCBIdx++;
		leftSphereRenderItem->pGeometry = m_Geometries["shapeGeo"].get();
		leftSphereRenderItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRenderItem->nIdxCnt = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].IndexCount;
		leftSphereRenderItem->nStartIdxLocation = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRenderItem->nBaseVertexLocation = leftSphereRenderItem->pGeometry->DrawArgs["sphere"].BaseVertexLocation;
		m_allRenderItems.push_back(std::move(leftSphereRenderItem));

		XMStoreFloat4x4(&rightSphereRenderItem->World, leftCylWorld);
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

// ������Ʈ�� ��� ���۸� �����Ѵ�.
inline void FroKEngine::UpdateObjectDBs(float fDeltaTime)
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

