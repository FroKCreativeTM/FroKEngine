#include "pch.h"
#include "Engine.h"
#include "Material.h"
#include "Transform.h"
#include "Light.h"

// �Ŵ�����
#include "Timer.h"
#include "Input.h"
#include "Resources.h"
#include "SceneManager.h"
#include "CollisionManager.h"
#include "InstancingManager.h"
#include "Audio.h"

void Engine::Init(const WindowInfo& info)
{
	_window = info;

	// �׷��� ȭ�� ũ�⸦ �����Ѵ�.
	_viewport = { 0, 0, static_cast<FLOAT>(info.width), static_cast<FLOAT>(info.height), 0.0f, 1.0f };
	_scissorRect = CD3DX12_RECT(0, 0, info.width, info.height);

	_device = make_shared<Device>();
	_graphicsCmdQueue = make_shared<GraphicsCommandQueue>();
	_computeCmdQueue = make_shared<ComputeCommandQueue>();
	_swapChain = make_shared<SwapChain>();
	_rootSignature = make_shared<RootSignature>();
	_graphicsDescHeap = make_shared<GraphicsDescriptorHeap>();
	_computeDescHeap = make_shared<ComputeDescriptorHeap>();

	_device->Init();
	_graphicsCmdQueue->Init(_device->GetDevice(), _swapChain);
	_computeCmdQueue->Init(_device->GetDevice());
	_swapChain->Init(_window, _device->GetDevice(), _device->GetDXGI(), _graphicsCmdQueue->GetCmdQueue());
	_rootSignature->Init();
	_graphicsDescHeap->Init(256);
	_computeDescHeap->Init();

	CreateConstantBuffer(CBV_REGISTER::b0, sizeof(LightParams), 1);	// ���� ���� ��� ��ü���� �ʾƼ� 1 
	CreateConstantBuffer(CBV_REGISTER::b1, sizeof(TransformParams), 256);
	CreateConstantBuffer(CBV_REGISTER::b2, sizeof(MaterialParams), 256);
	
	CreateRenderTargetGroups();

	ResizeWindow(info.width, info.height);

	GET_SINGLE(Input)->Init(info.hwnd);
	GET_SINGLE(Timer)->Init();
	GET_SINGLE(Resources)->Init();
	// GET_SINGLE(Audio)->Init();
}

void Engine::Update()
{
	GET_SINGLE(Input)->Update();
	GET_SINGLE(Timer)->Update();
	GET_SINGLE(SceneManager)->Update();
	GET_SINGLE(InstancingManager)->ClearBuffer();

	GET_SINGLE(CollisionManager)->Update();

	Render();

	ShowFps();
}

void Engine::Render()
{
	RenderBegin();

	// ��ü�� ���⼭ �׸���.
	GET_SINGLE(SceneManager)->Render();

	RenderEnd();
}

void Engine::ResizeWindow(int32 width, int32 height)
{
	//assert(_device);
	//assert(_swapChain);
	//assert(_graphicsCmdQueue);
	//assert(_computeCmdQueue);

	//// FlushCommandQueue
	//_graphicsCmdQueue->WaitSync();
	//_computeCmdQueue->WaitSync();

	//// Ŀ�ǵ� ����Ʈ���� ���� ���� ��Ų��.
	//_graphicsCmdQueue->Reset();
	//_computeCmdQueue->Reset();

	//_swapChain->Reset();

	_window.width = width;
	_window.height = height;

	RECT rect = { 0, 0, width, height };
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	::SetWindowPos(_window.hwnd, 0, 100, 100, width, height, 0);

	/*
    // �ϳ��� �ϼ����� �ʾҴٸ� ������ �߻��� ���̴�.
    assert(m_d3dDevice);
    assert(m_SwapChain);
    assert(m_DirectCmdListAlloc);

    // �ڿ��� ��ȭ��Ű�� ������ flush�� �ʿ��ϴ�.
    FlushCommandQueue();

    // Ŀ�ǵ� ����Ʈ�� ���� ���½�Ų��.
    ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

    // �츮�� �ٽ� ������ ���� �ڿ��� ���� �����Ѵ�.
    for (int i = 0; i < SwapChainBufferCount; ++i)
    {
        m_SwapChainBuffer[i].Reset();
    }
    // Depth/Stencil ���۸� �����Ѵ�.
    m_DepthStencilBuffer.Reset();

    // ���� ü���� ũ�⸦ �ٽ� �����Ѵ�.
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        SwapChainBufferCount,
        m_tRS.nWidth, m_tRS.nHeight,
        m_BackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    // ���� �� ���۸� 0���� �ʱ�ȭ �Ѵ�.
    m_CurrBackBuffer = 0;

    // Render targer view ������ ���� �����ͼ� �ڵ鿡 �����Ѵ�.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; i++)
    {
        // Swap chain�� i��° ���۸� �����´�.
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
        // �� ���ۿ� ���� Render Target View�� �����Ѵ�.
        m_d3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        // ���� ���� �׸����� �Ѿ��.
        rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
    }

    // depth/stencil ���ۿ� �並 �����Ѵ�.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_tRS.nWidth;
    depthStencilDesc.Height = m_tRS.nHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = m_DepthStencilFormat;
    depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // D3D12_CLEAR_VALUE�� Ư�� �ڿ��� ����� ����(�� ���� �ʱ�ȭ)�� 
    // �̸� �����ϱ� ���� Ÿ���̴�.
    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

    // �� ������ 0�� ��ü ���ҽ� ������ ����Ͽ� ������ ����
    m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, GetDepthStencilView());

    // ���ҽ��� �ʱ�ȭ �� ���¿��� Depth ���۸� ����ϵ��� ��ȯ�Ѵ�.
    // CD3DX12_RESOURCE_BARRIER�� GPU�� �ڿ��� �� ������� �ʾҰų�
    // ������� �������� �ʾ��� �� �̿� �����ϴ� �� �� Resource Hazard�� �����Ѵ�.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // �������� ����� �����Ѵ�.
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    // _countof(cmdsLists) : �迭�� �ִ� ��� ��� ��
    // cmdsLists : ��� ��ϵ��� �迭�� ù ���Ҹ� ����Ű�� ������
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // ������¡�� �Ϸ�� ������ ��ٸ���.
    FlushCommandQueue();

    // ����Ʈ ��ȯ�� Ŭ���̾�Ʈ ������ �����ϵ���  ������Ʈ �Ѵ�.
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_tRS.nWidth);
    m_ScreenViewport.Height = static_cast<float>(m_tRS.nHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = { 0, 0, static_cast<long>(m_tRS.nWidth), static_cast<long>(m_tRS.nHeight) };
    
    GET_SINGLE(SceneManager)->OnResize();
	*/
}

void Engine::RenderBegin()
{
	_graphicsCmdQueue->RenderBegin();
}

void Engine::RenderEnd()
{
	_graphicsCmdQueue->RenderEnd();
}

void Engine::ShowFps()
{
	uint32 fps = GET_SINGLE(Timer)->GetFps();

	WCHAR text[100] = L"";
	::wsprintf(text, L"FPS : %d", fps);

	::SetWindowText(_window.hwnd, text);
}

void Engine::CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count)
{
	uint8 typeInt = static_cast<uint8>(reg);
	assert(_constantBuffers.size() == typeInt);

	shared_ptr<ConstantBuffer> buffer = make_shared<ConstantBuffer>();
	buffer->Init(reg, bufferSize, count);
	_constantBuffers.push_back(buffer);
}

void Engine::CreateRenderTargetGroups()
{
	// DepthStencil
	shared_ptr<Texture> dsTexture = GET_SINGLE(Resources)->CreateTexture(L"DepthStencil",
		DXGI_FORMAT_D32_FLOAT, _window.width, _window.height,
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	// SwapChain Group
	{
		vector<RenderTarget> rtVec(SWAP_CHAIN_BUFFER_COUNT);

		for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		{
			wstring name = L"SwapChainTarget_" + std::to_wstring(i);

			ComPtr<ID3D12Resource> resource;
			_swapChain->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&resource));
			rtVec[i].target = GET_SINGLE(Resources)->CreateTextureFromResource(name, resource);
		}

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTexture);
	}

	// Shadow Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_SHADOW_GROUP_MEMBER_COUNT);

		// 4096 * 4096 size(�ؽ�ó ũ�Ⱑ Ŭ���� ������ �׸��ڰ� �����ȴ�.)
		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"ShadowTarget",
			DXGI_FORMAT_R32_FLOAT, 4096, 4096,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		shared_ptr<Texture> shadowDepthTexture = GET_SINGLE(Resources)->CreateTexture(L"ShadowDepthStencil",
			DXGI_FORMAT_D32_FLOAT, 4096, 4096,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SHADOW)]->Create(RENDER_TARGET_GROUP_TYPE::SHADOW, rtVec, shadowDepthTexture);
	}

	// Deferred Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT);

		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"PositionTarget",
			DXGI_FORMAT_R32G32B32A32_FLOAT, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"NormalTarget",
			DXGI_FORMAT_R32G32B32A32_FLOAT, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[2].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::G_BUFFER)]->Create(RENDER_TARGET_GROUP_TYPE::G_BUFFER, rtVec, dsTexture);
	}

	// Lighting Group
	{
		vector<RenderTarget> rtVec(RENDER_TARGET_LIGHTING_GROUP_MEMBER_COUNT);

		rtVec[0].target = GET_SINGLE(Resources)->CreateTexture(L"DiffuseLightTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		rtVec[1].target = GET_SINGLE(Resources)->CreateTexture(L"SpecularLightTarget",
			DXGI_FORMAT_R8G8B8A8_UNORM, _window.width, _window.height,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)] = make_shared<RenderTargetGroup>();
		_rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::LIGHTING)]->Create(RENDER_TARGET_GROUP_TYPE::LIGHTING, rtVec, dsTexture);
	}
}
