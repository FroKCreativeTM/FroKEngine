#include "WaveSimulatorScene.h"
#include "SceneManager.h"
#include "Layer.h"
#include "../Graphics/Camera.h"
#include "../Resource/ResourceManager.h"
#include "../Graphics/UploadBuffer.h"
#include "../Object/StaticObj/MeshObject.h"
#include "../Graphics/GeometryGenerator.h"
#include "../Graphics/Material.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture/Texture.h"
#include "../Wave.h"
#include "../TreeSpriteVertex.h"
#include "../InputManager.h"

WaveSimulatorScene::WaveSimulatorScene()
{
}

WaveSimulatorScene::~WaveSimulatorScene()
{
	Safe_Release_VecList(m_allRenderItems);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> WaveSimulatorScene::GetStaticSamplers()
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

void WaveSimulatorScene::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(
			GET_SINGLE(Core)->GetDevice().Get(), 1, (UINT)m_allRenderItems.size(),
			(UINT)m_Materials.size(), m_Waves->VertexCount()));
	}
}

void WaveSimulatorScene::LoadTexture()
{
	GET_SINGLE(ResourceManager)->LoadTexture("grassTex", L"Graphics/Texture/Datas/grass.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("waterTex", L"Graphics/Texture/Datas/water1.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("fenceTex", L"Graphics/Texture/Datas/WireFence.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("treeArrayTex", L"Graphics/Texture/Datas/treeArray2.dds");
}

void WaveSimulatorScene::BuildDescriptorHeaps()
{
	//
	// SRV 힙을 생성한다.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)));

	//
	// 힙이 할 일을 적어낸다.
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
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// 다음 서술자를 가져온다.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// 다음 서술자를 가져온다.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// 다음 서술자를 가져온다.
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	// 이 부분은 Texture2D 배열이기 때문에
	// 다른 방식으로 서술을 진행해줘야 한다.
	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;	// 딱히 밉맵 안 쓸꺼임
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);
}

void WaveSimulatorScene::BuildRootSignature()
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

	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void WaveSimulatorScene::BuildShadersAndInputLayout()
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
	GET_SINGLE(ResourceManager)->LoadShader("standardVS", L"Graphics\\Shader\\BlendDefault.hlsl", nullptr, "VS", "vs_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("opaquePS", L"Graphics\\Shader\\BlendDefault.hlsl", defines, "PS", "ps_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("alphaTestedPS", L"Graphics\\Shader\\BlendDefault.hlsl", alphaTestDefines, "PS", "ps_5_1");

	// 지오메트리 셰이더도 사용하자.
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

void WaveSimulatorScene::BuildWavesGeometryBuffers()
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

	auto geo = new MeshGeometry();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

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

void WaveSimulatorScene::BuildMaterials()
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

void WaveSimulatorScene::BuildLandGeometry()
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

	auto geo = new MeshGeometry();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

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

void WaveSimulatorScene::BuildBoxGeometry()
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

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

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

void WaveSimulatorScene::BuildTreeSpritesGeometry()
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

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(GET_SINGLE(Core)->GetDevice().Get(),
		GET_SINGLE(Core)->GetCommandList().Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

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

void WaveSimulatorScene::BuildRenderItems()
{
	// 레이어 생성
	Layer* pLayer = FindLayer("Default");

	MeshObject* pWaveObject = Object::CreateObj<MeshObject>("Water", pLayer);
	pWaveObject->SetWorldMatrix(MathHelper::Identity4x4());
	pWaveObject->SetTexTransform(XMMatrixScaling(5.0f, 5.0f, 1.0f));
	pWaveObject->SetObjCBIdx(0);
	pWaveObject->SetMaterial(m_Materials["water"]);
	pWaveObject->SetGeometry(m_Geometries["waterGeo"]);
	pWaveObject->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pWaveObject->SetIdxCnt(pWaveObject->GetGeometry()->DrawArgs["grid"].IndexCount);
	pWaveObject->SetStartIdxLocation(pWaveObject->GetGeometry()->DrawArgs["grid"].StartIndexLocation);
	pWaveObject->SetBaseVertexLocation(pWaveObject->GetGeometry()->DrawArgs["grid"].BaseVertexLocation);

	MeshObject* pGridObject = Object::CreateObj<MeshObject>("Grass", pLayer);
	pGridObject->SetWorldMatrix(MathHelper::Identity4x4());
	pGridObject->SetTexTransform(XMMatrixScaling(5.0f, 5.0f, 1.0f));
	pGridObject->SetObjCBIdx(1);
	pGridObject->SetMaterial(m_Materials["grass"]);
	pGridObject->SetGeometry(m_Geometries["landGeo"]);
	pGridObject->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pGridObject->SetIdxCnt(pGridObject->GetGeometry()->DrawArgs["grid"].IndexCount);
	pGridObject->SetStartIdxLocation(pGridObject->GetGeometry()->DrawArgs["grid"].StartIndexLocation);
	pGridObject->SetBaseVertexLocation(pGridObject->GetGeometry()->DrawArgs["grid"].BaseVertexLocation);

	MeshObject* pBoxObject = Object::CreateObj<MeshObject>("Box", pLayer);
	pBoxObject->SetWorldMatrix(MathHelper::Identity4x4());
	pBoxObject->SetTexTransform(XMMatrixScaling(3.0f, 2.0f, -9.0f));
	pBoxObject->SetObjCBIdx(2);
	pBoxObject->SetMaterial(m_Materials["wirefence"]);
	pBoxObject->SetGeometry(m_Geometries["boxGeo"]);
	pBoxObject->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pBoxObject->SetIdxCnt(pBoxObject->GetGeometry()->DrawArgs["box"].IndexCount);
	pBoxObject->SetStartIdxLocation(pBoxObject->GetGeometry()->DrawArgs["box"].StartIndexLocation);
	pBoxObject->SetBaseVertexLocation(pBoxObject->GetGeometry()->DrawArgs["box"].BaseVertexLocation);

	MeshObject* pTreeSpriteObject = Object::CreateObj<MeshObject>("TreeSprites", pLayer);
	pTreeSpriteObject->SetWorldMatrix(MathHelper::Identity4x4());
	pTreeSpriteObject->SetObjCBIdx(3);
	pTreeSpriteObject->SetMaterial(m_Materials["treeSprites"]);
	pTreeSpriteObject->SetGeometry(m_Geometries["treeSpritesGeo"]);
	pTreeSpriteObject->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pTreeSpriteObject->SetIdxCnt(pTreeSpriteObject->GetGeometry()->DrawArgs["points"].IndexCount);
	pTreeSpriteObject->SetStartIdxLocation(pTreeSpriteObject->GetGeometry()->DrawArgs["points"].StartIndexLocation);
	pTreeSpriteObject->SetBaseVertexLocation(pTreeSpriteObject->GetGeometry()->DrawArgs["points"].BaseVertexLocation);

	m_allRenderItems.push_back(std::move(pWaveObject));
	m_allRenderItems.push_back(std::move(pGridObject));
	m_allRenderItems.push_back(std::move(pBoxObject));
	m_allRenderItems.push_back(std::move(pTreeSpriteObject));
}

void WaveSimulatorScene::BuildPSO()
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
	opaquePsoDesc.RTVFormats[0] = GET_SINGLE(Core)->GetBackBufferFormat();
	opaquePsoDesc.SampleDesc.Count = GET_SINGLE(Core)->Get4xMsaaState() ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = GET_SINGLE(Core)->Get4xMsaaState() ? (GET_SINGLE(Core)->Get4xMsaaQuality() - 1) : 0;
	opaquePsoDesc.DSVFormat = GET_SINGLE(Core)->GetDepthStencilFormat();
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

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
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(
		&transparentPSODesc, IID_PPV_ARGS(&m_PSOs["transparent"])));

	// 알파 판정을 위한 PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestPSODesc = opaquePsoDesc;
	alphaTestPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(GET_SINGLE(ResourceManager)->FindShader("alphaTestedPS")->GetBufferPointer()),
		GET_SINGLE(ResourceManager)->FindShader("alphaTestedPS")->GetBufferSize()
	};
	alphaTestPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(
		&alphaTestPSODesc, IID_PPV_ARGS(&m_PSOs["alphaTested"])));


	// 
	// 트리를 위한 PSO 코드입니다.
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

	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&treeSpritePsoDesc,
		IID_PPV_ARGS(&m_PSOs["treeSprites"])));


	//
	// 불투명한 와이어프레임 개체에 대한 PSO입니다.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["opaque_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentWireframePsoDesc = transparentPSODesc;
	transparentWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&transparentWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["transparent_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaframePsoDesc = alphaTestPSODesc;
	alphaframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&alphaframePsoDesc, IID_PPV_ARGS(&m_PSOs["alpha_wireframe"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpriteWireframePsoDesc = treeSpritePsoDesc;
	treeSpriteWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateGraphicsPipelineState(&treeSpriteWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["treeSprite_wireframe"])));
}

void WaveSimulatorScene::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<MeshObject*>& ritems)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_curFrameResource->ObjectCB->Resource();
	auto matCB = m_curFrameResource->MaterialCB->Resource();

	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->GetGeometry()->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->GetGeometry()->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->GetPrimitiveType());

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

bool WaveSimulatorScene::Init()
{
	if (!Scene::Init())
	{
		return false;
	}

	// 먼저 텍스처를 불러온다. 
	LoadTexture();

	// 루트 시그네처나 
	// 서술자 힙 그리고 셰이더를 불러온다.
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();

	// 지오메트리 생성
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildBoxGeometry();
	BuildTreeSpritesGeometry();

	// 재질(머터리얼)을 불러온다.
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSO();

	return true;
}

void WaveSimulatorScene::OnMouseDown(int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void WaveSimulatorScene::OnMouseMove(int x, int y)
{

	if (GET_SINGLE(InputManager)->KeyDown("MouseLButton"))
	{
		// 각 픽셀이 4분의 1도에 해당하도록 합니다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		GET_SINGLE(Camera)->Pitch(dy);
		GET_SINGLE(Camera)->RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void WaveSimulatorScene::OnMouseUp(int x, int y)
{
}
