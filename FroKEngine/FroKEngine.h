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

	// 패스별 상수와 오브젝트 상수들을 저장하기 위한 버퍼
	std::unique_ptr<UploadBuffer<PassConstants>> m_PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;
	UINT m_passCbvOffset = 0;

	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	// 셰이더들을 상수로 매핑해서 저장한다.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// 프레임 리소스를 저장하기 위한 vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	// 모든 렌더 아이템의 리스트.
	std::vector<std::unique_ptr<RenderItem>> m_allRenderItems;
	std::vector<RenderItem*> m_OpaqueRenderItems;
	
	// 모든 마테리얼 정보
	std::unordered_map<std::string, std::unique_ptr<Material>> m_Material;

	bool m_IsWireframe = true;

	// 패스 상수들을 저장해둔 구조체
	PassConstants m_tMainPassCB;

	// 지오메트리 정보를 저장하기 위한 map
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

	// 먼저 커맨드 리스트를 초기화 한다.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildConstantBufferViews();
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

inline void FroKEngine::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, (UINT)m_allRenderItems.size()));
	}
}

// 서술자 힙을 생성한다.
// 이는 자원을 렌더링 파이프라인에 묶을 때 사용할 것입니다.
// Input : void
// Output : void
inline void FroKEngine::BuildDescriptorHeaps()
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
inline void FroKEngine::BuildConstantBufferViews()
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
inline void FroKEngine::BuildRootSignature()
{
	// 셰이더 프로그램은 일반적으로 입력으로 리소스를 필요로 합니다(상수 버퍼, 텍스처, 샘플러).
	// 루트 서명은 셰이더 프로그램이 기대하는 리소스를 정의합니다.
	// 셰이더 프로그램을 함수로, 입력 리소스를 함수 매개변수로 생각하면
	// 루트 서명은 함수 서명을 정의하는 것으로 생각할 수 있습니다.

	// 루트 매개변수는 테이블, 루트 설명자 또는 루트 상수일 수 있습니다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	// CBV의 단일 서술자 테이블을 만듭니다.
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// 테이블의 서술자 갯수
		0);	// 이 루트 매개변수에 묶일 셰이더 인수들의 기준 레지스터 번호( register (b0) )

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// 테이블의 서술자 갯수
		1);	// 이 루트 매개변수에 묶일 셰이더 인수들의 기준 레지스터 번호( register (b1) )

	slotRootParameter[0].InitAsDescriptorTable(1,	// 구간(range) 갯수
		&cbvTable0);	// 구간들의 배열을 가리키는 포인터
	slotRootParameter[1].InitAsDescriptorTable(1,	// 구간(range) 갯수
		&cbvTable1);	// 구간들의 배열을 가리키는 포인터

	// 루트 서명은 루트 매개변수의 배열입니다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
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
	m_shaders["standardVS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtil::CompileShader(L"Graphics\\Shader\\color.hlsl", nullptr, "PS", "ps_5_1");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

// 박스 지오메트리를 생성한다.
inline void FroKEngine::BuildBoxGeometry()
{
	// 여긴 딱히 할 일 없으니 냄긴다.
}

inline void FroKEngine::BuildShapeGeometry()
{
	GeometryGenerator geoGenerator;
	GeometryGenerator::MeshData box = geoGenerator.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGenerator.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGenerator.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGenerator.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	// 이 코드는 모든 기하 구조를 하나의 커다란 정점/인덱스 버퍼에 담는다.
	// 따라서 버퍼에서 각 부분 메시가 차지하는 영역들을 정의할 필요가 있다.

	// 연결된 정점 버퍼에서의 각 물체의 시작 인덱스를 적절한 변수들에 보관합니다.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// 연결된 인덱스 버퍼에서의 각 물체의 시작 인덱스를 적절한 변수들에 저장해둡니다.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// 정점 / 인덱스 버퍼에서 각 물체가 차지하는 영역을 나타내는 SubmeshGeometry 객체를 정의한다.
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

	// 필요한 정점 성분들을 추출하고, 모든 메시의 정점들을 하나의 정점 버퍼로 넣는다.

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

	// GPU에서 사용할 버퍼를 만둔다.
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

	// 기둥들과 구를 두 줄로 배치한다.
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
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};

	// 래스터라이즈 부분은 셰이더를 직접 프로그래밍할 수 없고
	// 단순히 설정만 가능한 부분이다.
	// 레스터나 그 외 설정들을 묶어줍니다.
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX; // 다중표본화를 설정합니다.(여기서는 그 어떤 표본도 비활성화하지 않는 MAX를 넣습니다.)
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// 기본 도형은 삼각형입니다
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

	// 각 렌더링할 아이템마다 파이프라인에 묶는다
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->pGeometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->pGeometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->primitiveType);

		// 이 개체 및 이 프레임 리소스에 대한 설명자 힙의 CBV에 대한 오프셋.
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

// 오브젝트의 상수 버퍼를 갱신한다.
inline void FroKEngine::UpdateObjectCBs(float fDeltaTime)
{
	auto curObjectCB = m_curFrameResource->ObjectCB.get();

	// 상수들이 바뀌었을 때에만 cbuffer 자료를 갱신해야 한다.
	// 이러한 갱신을 프레임 자원마다 수행해야 한다.
	for (auto & e : m_allRenderItems)
	{
		XMMATRIX world = XMLoadFloat4x4(&e->World);

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		curObjectCB->CopyData(e->objCBIdx, objConstants);

		// 다음 프레임 자원으로 넘어간다.
		e->nFramesDirty--;
	}
}

inline void FroKEngine::UpdateMaterialCBs(float fDeltaTime)
{
	auto curMaterialCB = m_curFrameResource->MaterialCB.get();

	for (auto& e : m_Material)
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

	// 상수 버퍼 뷰 서술자 힙을 가져옵니다.
	// 이는 렌더링 파이프라인에 자원을 묶을 때 사용합니다.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 그래픽스 루트 서명을 설정합니다.
	// SetGraphicsRootSignature을 이용하면 서술자 테이블을 가져와서 파이프라인에 묶을 수 있습니다.
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

	// 프레임 명령이 완료될 때까지 기다립니다. 
	// 이 대기는 비효율적이며 단순성을 위해서 있는 코드입니다. 나중에 렌더링 코드를 구성하는 방법을 보여줍니다.
	// 따라서 프레임당 기다릴 필요가 없습니다.
	// FlushCommandQueue();
}

