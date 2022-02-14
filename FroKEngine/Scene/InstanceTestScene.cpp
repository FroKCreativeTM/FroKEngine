#include "InstanceTestScene.h"
#include "../Object/StaticObj/MeshObject.h"
#include "../Graphics/Camera.h"
#include "../Resource/ResourceManager.h"
#include "../InputManager.h"
#include "../Graphics/Texture/Texture.h"


bool InstanceTestScene::Init()
{
    if (!Scene::Init())
    {
        return false;
    }

    // 먼저 커맨드 리스트를 초기화 한다.
    ThrowIfFailed(GET_SINGLE(Core)->GetCommandList()->Reset(GET_SINGLE(Core)->GetDirectCmdListAlloc().Get(), nullptr));

    // 이 힙 유형에서 설명자의 증분 크기를 가져옵니다. 
    // 이것은 하드웨어에 따라 다르므로 이 정보를 쿼리해야 합니다.
    m_CbvSrvDescriptorSize = GET_SINGLE(Core)->GetDevice()
        ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    GET_SINGLE(Camera)->SetPosition(0.0f, 2.0f, -15.0f);

    LoadTextures();
    BuildRootSignature();
    BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildSkullGeometry();
    BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // 초기화 명령을 실행합니다.
    ThrowIfFailed(GET_SINGLE(Core)->GetCommandList()->Close());
    ID3D12CommandList* cmdsLists[] = { GET_SINGLE(Core)->GetCommandList().Get() };
    GET_SINGLE(Core)->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // 초기화가 완료될 때까지 기다립니다.
    GET_SINGLE(Core)->FlushCommandQueue();

    return true;
}

void InstanceTestScene::Input(float fDeltaTime)
{
	Scene::Input(fDeltaTime);

	if (GET_SINGLE(InputManager)->KeyPress("Escape"))
	{
		GET_SINGLE(Core)->SetWindowLoop(false);
		PostQuitMessage(0);
	}

	if (GET_SINGLE(InputManager)->KeyPress("MoveFront"))
	{
		GET_SINGLE(Camera)->Walk(20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyPress("MoveBack"))
	{
		GET_SINGLE(Camera)->Walk(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyPress("MoveLeft"))
	{
		GET_SINGLE(Camera)->Strafe(-20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyPress("MoveRight"))
	{
		GET_SINGLE(Camera)->Strafe(20.0f * fDeltaTime);
	}
	if (GET_SINGLE(InputManager)->KeyDown("MouseLButton"))
	{
		m_LastMousePos.x = GET_SINGLE(InputManager)->GetMouseX();
		m_LastMousePos.y = GET_SINGLE(InputManager)->GetMouseY();
	}
	if (GET_SINGLE(InputManager)->KeyPress("MouseLButton"))
	{
		// 각 픽셀이 4분의 1도에 해당하도록 합니다.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(GET_SINGLE(InputManager)->GetMouseX() - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(GET_SINGLE(InputManager)->GetMouseY() - m_LastMousePos.y));

		GET_SINGLE(Camera)->Pitch(dy);
		GET_SINGLE(Camera)->RotateY(dx);

		m_LastMousePos.x = GET_SINGLE(InputManager)->GetMouseX();
		m_LastMousePos.y = GET_SINGLE(InputManager)->GetMouseY();
	}
}

int InstanceTestScene::Update(float fDeltaTime)
{
	return 0;
}

void InstanceTestScene::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	GET_SINGLE(Camera)->SetLens(0.25f * MathHelper::Pi, GET_SINGLE(Core)->AspectRatio(), 1.0f, 1000.0f);
	BoundingFrustum::CreateFromMatrix(mCamFrustum, GET_SINGLE(Camera)->GetProj());
}

void InstanceTestScene::OnResize()
{
}

void InstanceTestScene::AnimateMaterials(float fDeltaTime)
{
}

void InstanceTestScene::UpdateInstanceData(float fDeltaTime)
{
}

void InstanceTestScene::UpdateMaterialBuffer(float fDeltaTime)
{
}

void InstanceTestScene::UpdateMainPassCB(float fDeltaTime)
{
}

void InstanceTestScene::LoadTextures()
{
    GET_SINGLE(ResourceManager)->LoadTexture("bricksTex", L"bricks.dds");
    GET_SINGLE(ResourceManager)->LoadTexture("stoneTex", L"stone.dds");
    GET_SINGLE(ResourceManager)->LoadTexture("tileTex", L"tile.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("crateTex", L"WoodCrate01.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("iceTex", L"ice.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("grassTex", L"grass.dds");
	GET_SINGLE(ResourceManager)->LoadTexture("defaultTex", L"white1x1.dds");
}

void InstanceTestScene::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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

void InstanceTestScene::BuildDescriptorHeaps()
{

	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 7;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(GET_SINGLE(Core)->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	//
	// 실질적 디스크립터 정보를 이용해서 힙을 생성하자.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = GET_SINGLE(ResourceManager)->FindTexture("bricksTex")->pResource;
	auto stoneTex = GET_SINGLE(ResourceManager)->FindTexture("stoneTex")->pResource;
	auto tileTex = GET_SINGLE(ResourceManager)->FindTexture("tileTex")->pResource;
	auto crateTex =	GET_SINGLE(ResourceManager)->FindTexture("crateTex")->pResource;
	auto iceTex = GET_SINGLE(ResourceManager)->FindTexture("iceTex")->pResource;
	auto grassTex = GET_SINGLE(ResourceManager)->FindTexture("grassTex")->pResource;
	auto defaultTex = GET_SINGLE(ResourceManager)->FindTexture("defaultTex")->pResource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = stoneTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(stoneTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = tileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = crateTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = crateTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(crateTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = iceTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = iceTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(iceTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = grassTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// 다음 디스크립터로
	hDescriptor.Offset(1, m_CbvSrvDescriptorSize);

	srvDesc.Format = defaultTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = defaultTex->GetDesc().MipLevels;
	GET_SINGLE(Core)->GetDevice()->CreateShaderResourceView(defaultTex.Get(), &srvDesc, hDescriptor);
}

void InstanceTestScene::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	GET_SINGLE(ResourceManager)->LoadShader("standardVS", L"InstancingTest.hlsl", nullptr, "VS", "vs_5_1");
	GET_SINGLE(ResourceManager)->LoadShader("opaquePS", L"InstancingTest.hlsl", nullptr, "PS", "ps_5_1");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void InstanceTestScene::BuildSkullGeometry()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		// Project point onto unit sphere and generate spherical texture coordinates.
		XMFLOAT3 spherePos;
		XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		float theta = atan2f(spherePos.z, spherePos.x);

		// Put in [0, 2pi].
		if (theta < 0.0f)
			theta += XM_2PI;

		float phi = acosf(spherePos.y);

		float u = theta / (2.0f * XM_PI);
		float v = phi / XM_PI;

		vertices[i].TexC = { u, v };

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = new MeshGeometry;
	geo->Name = "skullGeo";

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
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.Bounds = bounds;

	geo->DrawArgs["skull"] = submesh;

	m_Geometries[geo->Name] = geo;
}

void InstanceTestScene::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
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
}

void InstanceTestScene::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(GET_SINGLE(Core)->GetDevice().Get(),
			1, (UINT)m_vecAllRenderObject.size(), (UINT)m_Materials.size()));
	}
}

void InstanceTestScene::BuildMaterials()
{
	auto bricks0 = GET_SINGLE(ResourceManager)->BuildMaterial("bricks0", 0, 0,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.02f, 0.02f, 0.02f), 0.1f);
	auto stone0 = GET_SINGLE(ResourceManager)->BuildMaterial("stone0", 1, 1,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.05f, 0.05f, 0.05f), 0.3f);
	auto tile0 = GET_SINGLE(ResourceManager)->BuildMaterial("tile0", 2, 2,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.02f, 0.02f, 0.02f), 0.3f);
	auto crate0 = GET_SINGLE(ResourceManager)->BuildMaterial("checkboard0", 3, 3,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.05f, 0.05f, 0.05f), 0.2f);
	auto ice0 = GET_SINGLE(ResourceManager)->BuildMaterial("ice0", 4, 4,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), 0.0f);
	auto grass0 = GET_SINGLE(ResourceManager)->BuildMaterial("grass0", 5, 5,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.05f, 0.05f, 0.05f), 0.2f);
	auto skullMat = GET_SINGLE(ResourceManager)->BuildMaterial("skullMat", 6, 6,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.05f, 0.05f, 0.05f), 0.5f);

	m_Materials["bricks0"] = bricks0;
	m_Materials["stone0"] = stone0;
	m_Materials["tile0"] = tile0;
	m_Materials["crate0"] = crate0;
	m_Materials["ice0"] = ice0;
	m_Materials["grass0"] = grass0;
	m_Materials["skullMat"] = skullMat;
}

void InstanceTestScene::BuildRenderItems()
{
	auto skullRitem = new SkullObject;
	skullRitem->World = MathHelper::Identity4x4();
	skullRitem->TexTransform = MathHelper::Identity4x4();
	skullRitem->ObjCBIndex = 0;
	skullRitem->Mat = mMaterials["tile0"].get();
	skullRitem->Geo = mGeometries["skullGeo"].get();
	skullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullRitem->InstanceCount = 0;
	skullRitem->IndexCount = skullRitem->Geo->DrawArgs["skull"].IndexCount;
	skullRitem->StartIndexLocation = skullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
	skullRitem->BaseVertexLocation = skullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;
	skullRitem->Bounds = skullRitem->Geo->DrawArgs["skull"].Bounds;

	// Generate instance data.
	const int n = 5;
	skullRitem->Instances.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				// Position instanced along a 3D grid.
				skullRitem->Instances[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&skullRitem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				skullRitem->Instances[index].MaterialIndex = index % mMaterials.size();
			}
		}
	}


	mAllRitems.push_back(std::move(skullRitem));

	// All the render items are opaque.
	for (auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}

void InstanceTestScene::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ritems)
{
}
