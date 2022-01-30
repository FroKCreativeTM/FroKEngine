#include "Core.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Core::GetInst()->WndProc(hwnd, msg, wParam, lParam);
}

// nullptr ������ ���⼭ �����ϴ�.
// �ֳĸ� ��� ���α׷� ���۰� ����� �ν��Ͻ���
// ���������ϱ�(static)
bool Core::m_bLoop = true;

Core* Core::m_pInst = nullptr;
Core* Core::GetInst()
{
	return m_pInst;
}

void Core::DestroyInst()
{
	SAFE_DELETE(m_pInst)
}

HINSTANCE Core::GetAppInst() const
{
    return m_hInst;
}

HWND Core::GetMainWnd() const
{
    return m_hWnd;
}

// ȭ��� ���ϱ�
float Core::AspectRatio() const
{
    return static_cast<float>(m_tRS.nWidth) / m_tRS.nHeight;
}

bool Core::Get4xMsaaState() const
{
    return m_4xMsaaState;
}

void Core::Set4xMsaaState(bool value)
{
    if (m_4xMsaaState != value)
    {
        m_4xMsaaState = value;

        // Recreate the swapchain and buffers with new multisample settings.
        CreateSwapChain();
        OnResize();
    }
}

bool Core::Init(HINSTANCE hInstance, int nWidth, int nHeight)
{
	// �ν��Ͻ� ������ �ھ �����մϴ�.
	m_hInst = hInstance;

	MyRegisterClass();

    // �ػ� ����
    m_tRS.nWidth = nWidth;
    m_tRS.nHeight = nHeight;

    Create();

    // �׷��ȿ� ���� ������ �Ѵ�.
    if (!InitDirect3D()) 
    {
        return false;
    }

    // �ʱ�ȭ �� �ڵ带 �̿��ؼ� ������� �Ѵ�.
    OnResize();

    // ���⿡ ���ӿ��� ����� �ϴ� �Ŵ������� �����Ѵ�.
    // Ÿ�̸�(FPS, ��ŸŸ��) �ʱ�ȭ
    if (!GET_SINGLE(Timer)->Init(m_hWnd))
    {
        return false;
    }

    // ��ǲ �Ŵ��� �ʱ�ȭ
    if (!GET_SINGLE(Input)->Init(m_hWnd, false))
    {
        return false;
    }

    // ��ΰ����� �ʱ�ȭ
    if (!GET_SINGLE(PathManager)->Init())
    {
        return false;
    }

    // ���ҽ� �Ŵ��� �ʱ�ȭ
    if (!GET_SINGLE(ResourceManager)->Init(m_d3dDevice.Get(), m_CommandList.Get()))
    {
        return false;
    }

    return true;
}

int Core::Run()
{
    MSG msg;

    GET_SINGLE(Timer)->Reset();

    ::ZeroMemory(&msg, sizeof(MSG));

    while (m_bLoop)
    {
        // ������ �޽��� ť�� ó���ϴ� �Լ��Դϴ�.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // ������ ����Ÿ��
        else
        {
            if (!m_AppPaused)
            {
                // ������ ������ ����ȴ�.
                Logic();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

ATOM Core::MyRegisterClass()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_FROKENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_MainWndCaption.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT Core::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ImGui(����)

    // ������ �޽���(Ű �Ŵ���)
    switch (message)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            GET_SINGLE(Timer)->Stop();
        }
        else
        {
            m_AppPaused = false;
            GET_SINGLE(Timer)->Start();
        }
        return 0;

        // ����ڰ� â�� ũ�⸦ ������ �� WM_SIZE�� ���۵˴ϴ�.
    case WM_SIZE:
        // �� Ŭ���̾�Ʈ ���� ������ �����մϴ�.
        m_tRS.nWidth = LOWORD(lParam);
        m_tRS.nHeight = HIWORD(lParam);
        if (m_d3dDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                // �ּ�ȭ ���·� ����?
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }

                // �ִ�ȭ ���·� ����?
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (m_Resizing)
                {
                    // ����ڰ� ũ�� ���� ���븦 �巡���ϴ� ���
                    // ����ڰ� ũ�� ���� ���븦 ��� ���� WM_SIZE �޽��� ��Ʈ���� â���� ���۵ǰ�
                    // �� WM_SIZE�� ���� ũ�⸦ �����ϴ� ���� 
                    // ���ǹ��ϰ� �������� ������ ���⿡�� ���� ũ�⸦ �������� �ʽ��ϴ�.
                    // ���� ��� ����ڰ� â ũ�� ������ �Ϸ��� �� �缳���ϰ� ũ�� ���� ���븦 �����Ͽ� 
                    // WM_EXITSIZEMOVE �޽����� �����ϴ�.
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

    // WM_EXITSIZEMOVE �޽����� ����ڰ� ũ�� ���� ���븦 ���� �� ���۵˴ϴ�.
    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        GET_SINGLE(Timer)->Stop();
        return 0;

    // WM_EXITSIZEMOVE ����ڰ� ũ�� ���� ���븦 ���� �� ���۵˴ϴ�.
    // ���⼭ �츮�� �� â ũ�⸦ ������� ��� ���� �缳���մϴ�.
    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        GET_SINGLE(Timer)->Start();
        OnResize();
        return 0;

    case WM_DESTROY:
        m_bLoop = false;
        PostQuitMessage(0);
        return 0;

    // WM_MENUCHAR �޽����� �޴��� Ȱ�� �����̰� ����ڰ� mnemonic �Ǵ�
    // ���� Ű�� �ش����� �ʴ� Ű�� ������ �� ���۵˴ϴ�.
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

    // â�� �ʹ� �۾����� �ʵ��� �� �޼����� ���� ����ϴ�.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_KEYDOWN: case WM_SYSKEYDOWN:
        GET_SINGLE(Input)->KeyDown(wParam);
        return 0;
    case WM_KEYUP: case WM_SYSKEYUP:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        else if ((int)wParam == VK_F2)
            // 4X MSAA
            Set4xMsaaState(!m_4xMsaaState);

        GET_SINGLE(Input)->KeyUp(wParam);
        return 0;
    case WM_CHAR:
        GET_SINGLE(Input)->KeyIn(wParam);
        return 0;
    case WM_MOUSEMOVE:
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseMove(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_INPUT:
        GET_SINGLE(Input)->MouseRawIn(lParam);
        return 0;
    case WM_LBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseLButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_LBUTTONUP:
        GET_SINGLE(Input)->SetMouseLButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_MBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseMButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_MBUTTONUP:
        GET_SINGLE(Input)->SetMouseMButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_RBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseRButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_RBUTTONUP:
        GET_SINGLE(Input)->SetMouseRButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_XBUTTONDOWN: case WM_XBUTTONUP:
        GET_SINGLE(Input)->SetMouseXButton(wParam);
        GET_SINGLE(Input)->MouseUp();
        return 0;
    case WM_DEVICECHANGE:
        // ��Ʈ�ѷ�
        GET_SINGLE(Input)->CheckControllers();
        return 0;
    }

    // �⺻ ��!
    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL Core::Create()
{
    m_hWnd = CreateWindowW(
        m_MainWndCaption.c_str(),
        m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        0, 
        CW_USEDEFAULT, 
        0, 
        nullptr, nullptr, 
        m_hInst, nullptr);

    if (!m_hWnd)
    {
        return FALSE;
    }

    // Ÿ��Ʋ�ٳ� �����츦 ������ ũ�⸦ ����
    RECT rc = { 0, 0, m_tRS.nWidth, m_tRS.nHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // ������ ������ ũ��� ������ Ŭ���̾�Ʈ ũ�⸦ ���ϴ� ũ��� ������� �Ѵ�.
    SetWindowPos(m_hWnd, HWND_TOPMOST, 100, 100, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

    // �����츦 ������ ����.
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return TRUE;
}

void Core::Logic()
{
    // ���� ���� ������Ʈ �ؾߵǴ� ����
    GET_SINGLE(Timer)->Update();
    // �츮�� �Լ��� ����� �� ��ŸŸ�ӿ� �̰��� �����ϸ� �ȴ�.
    float fDeltaTime = GET_SINGLE(Timer)->GetDeltaTime();
    CalculateFrameStats();

    // �Է��� �޴´�.
    Input(fDeltaTime);
    Update(fDeltaTime);
    LateUpdate(fDeltaTime);
    Collision(fDeltaTime);

    Render(fDeltaTime);
}

void Core::CreateRtvAndDsvDescriptorHeaps()
{
    // Render Targer View�� ���� �����ڸ� �����Ѵ�.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

    // Depth/Stencil View�� ���� �����ڸ� �����ϰ� �ۼ��Ѵ�.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void Core::OnResize()
{
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
    m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, DepthStencilView());

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
    // m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

bool Core::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG) 
    // D3D12 ����� ��Ʈ�ѷ��� �Ҵ�.
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    // IID_PPV_ARGS�� COM�� ID �� ppType�� RIID�� void**�� ġȯ�Ѵ�.
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    // Direct3D 12 �ϵ���� ����̽��� �����Ѵ�.
    // ���� �ּ� ���� ������ Direct3D 11�� �Ѵ�.
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,             // default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_d3dDevice));

    // ���� �ϵ���� ����̽��� ���ٸ� WARP(������ ��� ������ȭ �÷���) ����̽� �� ����Ʈ����� ��ü�Ѵ�.
    if (FAILED(hardwareResult))
    {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3dDevice)));
    }


    ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_Fence)));

    /// <summary>
    /// �����ڴ� ������ ���� ����.
    /// �� �����ڵ鿡 ���� ����� ������ ���� �� ������ ũ�⸦ �����ͼ� ����� �����ؾ��Ѵ�.
    /// </summary>
    m_RtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    ///////////////////////////////////////
    //              ��� ����             //
    ///////////////////////////////////////
     
    // ������� 4X MSAA ����Ƽ ������ üũ�մϴ�.
    // Direct3D 11�� ������ ������ 4X MSAA�� ��� ���� Ÿ�� ���˿� �����մϴ�.
    // �׷��� ������ ����Ƽ ���� �κи� üũ�� �մϴ�.
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_BackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
    // ����� ����� ���(GPU)�� ���� �α׸� ����ϴ�.
    LogAdapters();
#endif

    // Ŀ�ǵ� ������Ʈ 
    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void Core::CreateCommandObjects()
{
    // Ŀ�ǵ� ť�� ���� �����ڸ� ä��� �����Ѵ�.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

    // Ŀ�ǵ� �Ҵ��ڸ� �����Ѵ�.
    // �� �Ҵ��ڴ� Ŀ�ǵ� ����Ʈ�� �޸𸮸� �Ҵ��ϱ� ���� �����Ѵ�.
    // ��� ��Ͽ� �߰��� ��ɵ��� �� �Ҵ����� �޸𸮿� ����ȴ�.
    ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, // GPU�� ���� �����ϴ� ��� ���
        IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));    // RIID�� void** Ÿ���� Ŀ�ǵ� �Ҵ��� COM ��ü�� �־��ش�.

    // Ŀ�ǵ� ť�� ���� Ŀ�ǵ� ����Ʈ�� �����Ѵ�.
    ThrowIfFailed(m_d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_DirectCmdListAlloc.Get(),     // ���õ� Ŀ�ǵ� �Ҵ��ڸ� �ִ´�.
        nullptr,                        // PipelineStateObject�� �ʱ�ȭ�Ѵ�.
        IID_PPV_ARGS(m_CommandList.GetAddressOf())));

    // ���� ���·� ������ �Ѵ�.
    // ó�� Ŀ�ǵ� ����Ʈ�� ������ �� �̸� ������ ���̴�.
    // �׸��� Ŀ�ǵ� ����Ʈ�� �����ϱ� ������ �����־�� �Ѵ�.
    m_CommandList->Close();
}

void Core::CreateSwapChain()
{
    // ����� �ϱ� ���� ������ �����Ѵ�.
    m_SwapChain.Reset();

    // ���� ü�ο� ���� ������ ���´�.
    // ���� ü���� DXGI�� �������̽��̴�.
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_tRS.nWidth;
    sd.BufferDesc.Height = m_tRS.nHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = m_hWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // ���� : ���� ü���� �÷��� �ϱ� ���ؼ� Ŀ�ǵ� ť�� ����մϴ�.
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
        m_CommandQueue.Get(),
        &sd,
        m_SwapChain.GetAddressOf()));
}

/// <summary>
/// Ŀ�ǵ� ť�� �÷����մϴ�.
/// </summary>
void Core::FlushCommandQueue()
{
    // �� �潺 ����Ʈ���� ����� ǥ���ϱ� ���� �潺���� ������Ų��.
    m_CurrentFence++;

    // ��� ��⿭�� ����� �߰��Ͽ� �� �潺 ����Ʈ�� �����Ѵ�.
    // �ֳ��ϸ� �츮�� GPU Ÿ�Ӷ��ο� �ִ� ��� GPU�� ������ �Ϸ�� ������ �� �潺 ����Ʈ�� �������� �ʽ��ϴ�. (ť�� ó���Ǵ� ����!)
    // �� Signal() ������ ��� ����� ó���մϴ�. (ť ���� ������ ���� ������ GPU�� ���� ��� ����� �����Ѵ�!)
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

    // GPU�� �� ��Ÿ�� �������� ����� �Ϸ��� ������ ��ٸ���.
    if (m_Fence->GetCompletedValue() < m_CurrentFence)
    {
        // �̺�Ʈ�� �ڵ��ϱ� ���� �ڵ� ��ü�� �ϳ� �����Ѵ�.
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

        // GPU�� ���� �潺�� �����ϸ� �׿� �´� �̺�Ʈ�� �߻���Ų��.
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

        // CPU�� ���� �潺�� �̺�Ʈ�� ����Ǵ� ���� ��ٸ���.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* Core::CurrentBackBuffer() const
{
    return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrBackBuffer,
        m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::DepthStencilView() const
{
    return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Core::CalculateFrameStats()
{
    // �ڵ�� �ʴ� ��� �������� ����ϰ�
    // �� �������� �������ϴ� �� �ɸ��� ��� �ð�. 
    // �̷��� ���� ������ ĸ�� ǥ���ٿ� �߰��˴ϴ�.

    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((GET_SINGLE(Timer)->GetTotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        std::wcout << L"fps: " + fpsStr + L"   mspf: " + mspfStr << std::endl;

        wstring windowText = m_MainWndCaption +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_hWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void Core::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutputs(adapterList[i]);
        ReleaseCom(adapterList[i]);
    }
}

void Core::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, m_BackBufferFormat);

        ReleaseCom(output);

        ++i;
    }
}

void Core::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // ����Ʈ ���� ������ �������� ���� nullptr�� �ִ´�.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        ::OutputDebugString(text.c_str());
    }
}

Core::Core()
{
    // Only one D3DApp can be constructed.
    assert(m_pInst == nullptr);
    m_pInst = this;

    // �޸� ������ üũ�ϱ� ���� ����
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // �޸� ���� ���� ���� ����.
    // �Ű������� �� �޸��� ��� ��ȣ�� �־��ָ�,
    // �޸� �� �κ��� �ٷ� �̵��Ѵ�. (ȣ�� ������ �� �� �� �־��..)
    // _CrtSetBreakAlloc(313);

    // ������ �ð��� üũ�ؼ� �̰� ���۽�ų�� ���� �����Ѵ�.
#ifdef _DEBUG   
    // �ܼ�â�� ���������ִ� �Լ�
    if (AllocConsole())
    {
        FILE* nfp[3];
        freopen_s(nfp + 0, "CONOUT$", "rb", stdin);
        freopen_s(nfp + 1, "CONOUT$", "wb", stdout);
        freopen_s(nfp + 2, "CONOUT$", "wb", stderr);
        std::ios::sync_with_stdio();
    }
#endif
}

Core::~Core()
{
    DESTROY_SINGLE(Input);
    DESTROY_SINGLE(Timer);
    DESTROY_SINGLE(PathManager);
    DESTROY_SINGLE(ResourceManager);

    if (m_d3dDevice != nullptr)
    {
        FlushCommandQueue();
    }

    // �ܼ�â ����
#ifdef _DEBUG
    FreeConsole();
#endif // _DEBUG
}
